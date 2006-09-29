<?php
/*
** ZABBIX
** Copyright (C) 2000-2005 SIA Zabbix
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/
?>
<?php
require_once "include/graphs.inc.php";
require_once "include/profiles.inc.php";
require_once "include/triggers.inc.php";
require_once "include/items.inc.php";

/* HOST GROUP functions */
	function	add_host_to_group($hostid, $groupid)
	{
		if(!is_numeric($hostid) || !is_numeric($groupid)){
			error("incorrect parameters for 'add_host_to_group'");
			return FALSE;
		}
		$hostgroupid=get_dbid("hosts_groups","hostgroupid");
		$result=DBexecute("insert into hosts_groups (hostgroupid,hostid,groupid) values ($hostgroupid,$hostid,$groupid)");
		if(!$result)
			return $result;
		return $hostgroupid;
	}

	function	db_save_group($name,$groupid=NULL)
	{
		if(!is_string($name)){
			error("incorrect parameters for 'db_save_group'");
			return FALSE;
		}
	
		if($groupid==NULL)
			$result = DBexecute("select * from groups where name=".zbx_dbstr($name));
		else
			$result = DBexecute("select * from groups where name=".zbx_dbstr($name).
				" and groupid<>$groupid");
		
		if(DBfetch($result))
		{
			error("Group '$name' already exists");
			return FALSE;
		}
		if($groupid==NULL)
		{
			$groupid=get_dbid("groups","groupid");
			if(!DBexecute("insert into groups (name) values (".zbx_dbstr($name).")"))
				return FALSE;
			return $groupid;

		}
		else
			return DBexecute("update groups set name=".zbx_dbstr($name)." where groupid=$groupid");
	}

	function	add_group_to_host($hostid,$newgroup="")
	{
		if($newgroup == "" || $newgroup == NULL)
			 return TRUE;

		$result = db_save_group($newgroup);
		if(!$result)
			return	$result;
		
		$groupid = DBinsert_id($result,"groups","groupid");

		return add_host_to_group($hostid, $groupid);
	}

	function	update_host_groups_by_groupid($groupid,$hosts=array())
	{
		DBexecute("delete from hosts_groups where groupid=$groupid");

		foreach($hosts as $hostid)
		{
			add_host_to_group($hostid, $groupid);
		}
	}

	function	update_host_groups($hostid,$groups=array())
	{
		DBexecute("delete from hosts_groups where hostid=$hostid");

		foreach($groups as $groupid)
		{
			add_host_to_group($hostid, $groupid);
		}
	}

	function	add_host_group($name,$hosts=array())
	{
		$result = db_save_group($name);
		if(!$result)
			return	$result;
		
		$groupid = DBinsert_id($result,"groups","groupid");

		update_host_groups_by_groupid($groupid,$hosts);

		return $groupid;
	}

	function	update_host_group($groupid,$name,$hosts)
	{
		$result = db_save_group($name,$groupid);
		if(!$result)
			return	$result;
		
		update_host_groups_by_groupid($groupid,$hosts);

		return $result;
	}

/* HOST finction */
	function 	check_circle_host_link($hostid, $templateid)
	{
		if($templateid <= 0)		return FALSE;
		if($hostid == $templateid)	return TRUE;
		$template = get_host_by_hostid($templateid);
		if($template["templateid"] > 0)
			return check_circle_host_link($hostid, $template["templateid"]);

		return FALSE;
	}

	function	db_save_host($host,$port,$status,$useip,$ip,$templateid,$hostid=NULL)
	{
 		if (!eregi('^([0-9a-zA-Z\_\.-]+)$', $host)) 
		{
			error("Hostname should contain 0-9a-zA-Z_.- characters only");
			return FALSE;
		}

		if($hostid==NULL)
			$result=DBexecute("select * from hosts where host=".zbx_dbstr($host));
		else
			$result=DBexecute("select * from hosts where host=".zbx_dbstr($host).
				" and hostid<>$hostid");

		if(DBfetch($result))
		{
			error("Host '$host' already exists");
			return FALSE;
		}

		if($useip=="on" || $useip=="yes" || $useip==1)		$useip=1;
		else							$useip=0;

		if($hostid==NULL)
		{
			$hostid = get_dbid("hosts","hostid");
			$result = DBexecute("insert into hosts".
				" (hostid,host,port,status,useip,ip,disable_until,available,templateid)".
				" values ($hostid,".zbx_dbstr($host).",$port,$status,$useip,".zbx_dbstr($ip).",0,"
				.HOST_AVAILABLE_UNKNOWN.",$templateid)");
			if($result) $result = $hostid;
		}
		else
		{
			if(check_circle_host_link($hostid, $templateid))
			{
				error("Circle link can't be created");
				return FALSE;
			}

			$result = DBexecute("update hosts set host=".zbx_dbstr($host).",".
				"port=$port,useip=$useip,ip=".zbx_dbstr($ip).",templateid=$templateid".
				" where hostid=$hostid");

			update_host_status($hostid, $status);
		}
		return $result;
	}

	function	add_host($host,$port,$status,$useip,$ip,$templateid,$newgroup,$groups)
	{
		$result = db_save_host($host,$port,$status,$useip,$ip,$templateid);
		if(!$result)
			return $result;
	
		$hostid = DBinsert_id($result,"hosts","hostid");

		update_host_groups($hostid,$groups);

		add_group_to_host($hostid,$newgroup);

		sync_host_with_templates($hostid);

		update_profile("HOST_PORT",$port);
		
		return	$hostid;
	}

	function	update_host($hostid,$host,$port,$status,$useip,$ip,$templateid,$newgroup,$groups)
	{
		$old_host = get_host_by_hostid($hostid);

		$result = db_save_host($host,$port,$status,$useip,$ip,$templateid,$hostid);
		if(!$result)
			return $result;

		update_host_groups($hostid, $groups);

		add_group_to_host($hostid,$newgroup);

		if($old_host["templateid"] != $templateid)
			sync_host_with_templates($hostid);

		return	$result;
	}

	function	unlink_template($hostid, $templateid = null, $unlink_mode = true)
	{
		delete_template_elements($hostid, $templateid = null, $unlink_mode);
		DBexecute("update hosts set templateid=0 where hostid=".$hostid);
	}

	function	delete_template_elements($hostid, $templateid = null, $unlink_mode = false)
	{
		delete_template_graphs($hostid, $templateid, $unlink_mode);
		delete_template_triggers($hostid, $templateid, $unlink_mode);
		delete_template_items($hostid, $templateid, $unlink_mode);
		delete_template_applications($hostid, $templateid, $unlink_mode);
	}	

	function	copy_template_elements($hostid, $templateid = null, $copy_mode = false)
	{
		copy_template_applications($hostid, $templateid, $copy_mode);
		copy_template_items($hostid, $templateid, $copy_mode);
		copy_template_triggers($hostid, $templateid, $copy_mode);
		copy_template_graphs($hostid, $templateid, $copy_mode);
	}

# Sync host with linked template
	function	sync_host_with_templates($hostid, $templateid = null)
	{
		delete_template_elements($hostid, $templateid);		
		copy_template_elements($hostid, $templateid);
	}

	function	delete_groups_by_hostid($hostid)
	{
		$sql="select groupid from hosts_groups where hostid=$hostid";
		$result=DBselect($sql);
		while($row=DBfetch($result))
		{
			$sql="delete from hosts_groups where hostid=$hostid and groupid=".$row["groupid"];
			DBexecute($sql);
			$sql="select count(*) as count from hosts_groups where groupid=".$row["groupid"];
			$result2=DBselect($sql);
			$row2=DBfetch($result2);
			if($row2["count"]==0)
			{
				$sql="delete from groups where groupid=".$row["groupid"];
				DBexecute($sql);
			}
		}
	}

	# Delete Host

	function	delete_host($hostid, $unlink_mode = false)
	{
		global $DB_TYPE;

		$ret = FALSE;

	// unlink child hosts
		$db_childs = get_hosts_by_templateid($hostid);
		while($db_child = DBfetch($db_childs))
		{
			unlink_template($db_child["hostid"], NULL, $unlink_mode);
		}

	// delete items -> triggers -> graphs
		$db_items = get_items_by_hostid($hostid);
		while($db_item = DBfetch($db_items))
		{
			delete_item($db_item["itemid"]);
		}

	// delete host from maps
		delete_sysmaps_elements_with_hostid($hostid);
		
	// delete host from group
		DBexecute("delete from hosts_groups where hostid=$hostid");

	// delete host profile
		delete_host_profile($hostid);

	// delete host
		return DBexecute("delete from hosts where hostid=$hostid");
	}

	function	delete_host_group($groupid)
	{
		$sql="delete from hosts_groups where groupid=$groupid";
		DBexecute($sql);
		$sql="delete from groups where groupid=$groupid";
		return DBexecute($sql);
	}

	function	get_hostgroup_by_groupid($groupid)
	{
		$result=DBselect("select * from groups where groupid=".$groupid);
		$row=DBfetch($result);
		if($row)
		{
			return $row;
		}
		error("No host groups with groupid=[$groupid]");
		return  FALSE;
	}

	function	get_host_by_itemid($itemid)
	{
		$sql="select h.* from hosts h, items i where i.hostid=h.hostid and i.itemid=$itemid";
		$result=DBselect($sql);
		$row=DBfetch($result);
		if($row)
		{
			return $row;
		}
		error("No host with itemid=[$itemid]");
		return	FALSE;
	}

	function	get_host_by_hostid($hostid,$no_error_message=0)
	{
		$sql="select * from hosts where hostid=$hostid";
		$result=DBselect($sql);
		$row=DBfetch($result);
		if($row)
		{
			return $row;
		}
		if($no_error_message == 0)
			error("No host with hostid=[$hostid]");
		return	FALSE;
	}

	function	get_hosts_by_templateid($templateid)
	{
		return DBselect("select * from hosts where templateid=$templateid");
	}

	# Update Host status

	function	update_host_status($hostid,$status)
	{
		$sql="select status,host from hosts where hostid=$hostid";
		$result=DBselect($sql);
		$row=DBfetch($result);
		$old_status=$row["status"];
		if($status != $old_status)
		{
			update_trigger_value_to_unknown_by_hostid($hostid);
			info("Updated status of host ".$row["host"]);
			return	DBexecute("update hosts set status=$status".
				" where hostid=$hostid and status!=".HOST_STATUS_DELETED);
		}
		else
		{
			return 1;
		}
	}
	
	function	get_template_path($hostid)
	{
		$host = get_host_by_hostid($hostid);

		if ($host["templateid"]==0)
			return "/";

		$tmp_host = get_host_by_hostid($host["templateid"]);	
		return get_template_path($tmp_host["hostid"]).$tmp_host["host"]."/";
	}

	function get_correct_group_and_host($a_groupid=NULL, $a_hostid=NULL, $perm=PERM_READ_WRITE, $options = array())
	{
		if(!is_array($options))
		{
			error("Incorrest options for get_correct_group_and_host");
			show_page_footer();
			exit;
		}

		global $USER_DETAILS;
		
		$first_hostid_in_group = 0;

		$allow_all_hosts = (in_array("allow_all_hosts",$options)) ? 1 : 0;
		$always_select_first_host = in_array("always_select_first_host",$options) ? 1 : 0;

		if(in_array("monitored_hosts",$options))
			$with_host_status = " and h.status=".HOST_STATUS_MONITORED;
		else
			$with_host_status = "";

		if(in_array("with_monitored_items",$options)){
			$item_table = ",items i";	$with_items = " and h.hostid=i.hostid and i.status=".ITEM_STATUS_ACTIVE;
		}elseif(in_array("with_items",$options)){
			$item_table = ",items i";	$with_items = " and h.hostid=i.hostid";
		} else {
			$item_table = "";		$with_items = "";
		}

		$accessed_hosts = get_accessible_hosts_by_userid($USER_DETAILS['userid'],$perm);

		if(is_null($a_groupid))
		{
			$groupid = 0;
		}
		else
		{
			$groupid = $a_groupid;

			if($groupid > 0)
			{
				if(!DBfetch(DBselect("select distinct g.groupid from groups g, hosts_groups hg, hosts h".$item_table.
					" where hg.groupid=g.groupid and h.hostid=hg.hostid and h.hostid in (".$accessed_hosts.") ".
					$with_host_status.$with_items)))
				{
					$groupid = 0;
				}
			}

		}
		if(is_null($a_hostid))
		{
			$hostid = 0;
		}
		else
		{
			$hostid = $a_hostid;
			if(!($hostid == 0 && $allow_all_hosts == 1)) /* is not 'All' selected */
			{
				$group_table = "";
				$witth_group = "";
				if($groupid != 0)
				{
					if(!DBfetch(DBselect("select hg.hostid from hosts_groups hg".
						" where hg.groupid=".$groupid." and hg.hostid=".$hostid)))
					{
						$hostid = 0;
					}
					$group_table = " ,hosts_groups hg ";
					$witth_group = " and hg.hostid=h.hostid and hg.groupid=".$groupid;
				}

				if($db_host = DBfetch(DBselect("select distinct h.hostid,h.host from hosts h ".$item_table.$group_table.
					" where h.hostid in (".$accessed_hosts.") "
					.$with_host_status.$with_items.$witth_group.
					" order by h.host")))
				{
					$first_hostid_in_group = $db_host["hostid"];
				}

				if($first_hostid_in_group == 0)	$hostid = 0; /* no hosts in selected grpore */

				if($hostid > 0)
				{
					if(!DBfetch(DBselect("select distinct h.hostid from hosts h".$item_table.
						" where h.hostid=".$hostid.$with_host_status.$with_items.
						" and h.hostid in (".$accessed_hosts.") ")))
					{
							$hostid = 0;
					}
				}

				if(($hostid < 0) || ($hostid == 0 && $always_select_first_host == 1)) /* incorrect host */
				{
					$hostid = $first_hostid_in_group;
				}
			}
		}

		$group_correct	= ($groupid == $a_groupid) ? 1 : 0;
		$host_correct	= ($hostid == $a_hostid) ? 1 : 0;
		return array(
			"groupid"	=> $groupid,
			"group_correct"	=> $group_correct,
			"hostid"	=> $hostid,
			"host_correct"	=> $host_correct,
			"correct"	=> ($group_correct && $host_correct) ? 1 : 0
			);
	}

	function	validate_group_with_host($perm, $options = array(),$group_var=NULL,$host_var=NULL)
	{
		if(is_null($group_var)) $group_var = "web.latest.groupid";
		if(is_null($host_var))	$host_var = "web.latest.hostid";

		$_REQUEST["groupid"]    = get_request("groupid",get_profile($group_var,0));
		$_REQUEST["hostid"]     = get_request("hostid",get_profile($host_var,
			(in_array("always_select_first_host",$options)) ? -1 : 0));

		$result = get_correct_group_and_host($_REQUEST["groupid"],$_REQUEST["hostid"], $perm, $options);

		$_REQUEST["groupid"]    = $result["groupid"];
		$_REQUEST["hostid"]     = $result["hostid"];

		update_profile($host_var,$_REQUEST["hostid"]);
		update_profile($group_var,$_REQUEST["groupid"]);
	}

	function	validate_group($perm, $options = array(),$group_var=NULL)
	{
		if(is_null($group_var)) $group_var = "web.latest.groupid";

		$_REQUEST["groupid"]    = get_request("groupid",get_profile($group_var,0));

		$result = get_correct_group_and_host($_REQUEST["groupid"],NULL,$perm,$options);

		$_REQUEST["groupid"]    = $result["groupid"];

		update_profile($group_var,$_REQUEST["groupid"]);
	}

/* APPLICATIONS */

	function	db_save_application($name,$hostid,$applicationid=NULL,$templateid=0)
	{
		if(!is_string($name)){
			error("incorrect parameters for 'db_save_application'");
			return FALSE;
		}
	
		if($applicationid==NULL)
			$result = DBexecute("select * from applications where name=".zbx_dbstr($name)." and hostid=".$hostid);
		else
			$result = DBexecute("select * from applications where name=".zbx_dbstr($name)." and hostid=".$hostid.
				" and applicationid<>$applicationid");

		$db_app = DBfetch($result);
		if($db_app && $templateid==0)
		{
			error("Application '$name' already exists");
			return FALSE;
		}
		if($db_app && $applicationid!=NULL)
		{ // delete old item with same name
			delete_application($db_app["applicationid"]);
		}

		if($db_app && $applicationid==NULL)
		{ // if found application with same name update them, adding not needed
			$applicationid = $db_app["applicationid"];
		}

		$host = get_host_by_hostid($hostid);
		
		if($applicationid==NULL)
		{
			$applicationid_new = get_dbid("applications","applicationid");
			if($result = DBexecute("insert into applications (applicationid,name,hostid,templateid)".
				" values ($applicationid_new,".zbx_dbstr($name).",$hostid,$templateid)"))
					info("Added new application ".$host["host"].":$name");
		}
		else
		{
			$old_app = get_application_by_applicationid($applicationid);
			if($result = DBexecute("update applications set name=".zbx_dbstr($name).",hostid=$hostid,templateid=$templateid".
                                " where applicationid=$applicationid"))
					info("Updated application ".$host["host"].":".$old_app["name"]);
		}

		if(!$result)	return $result;

		if($applicationid==NULL)
		{
			$applicationid = $applicationid_new;

			$db_childs = get_hosts_by_templateid($hostid);
			while($db_child = DBfetch($db_childs))
			{// recursion
				$result = add_application($name,$db_child["hostid"],$applicationid);
				if(!$result) break;
			}
		}
		else
		{
			$db_applications = get_applications_by_templateid($applicationid);
			while($db_app = DBfetch($db_applications))
			{// recursion
				$result = update_application($db_app["applicationid"],$name,$db_app["hostid"],$applicationid);
				if(!$result) break;
			}
		}

		if($result)
			return $applicationid;

		if($templateid == 0){
			delete_application($itemid);
		}

	}
	function	add_application($name,$hostid,$templateid=0)
	{
		return db_save_application($name,$hostid,NULL,$templateid);
	}

	function	update_application($applicationid,$name,$hostid,$templateid=0)
	{
		return db_save_application($name,$hostid,$applicationid,$templateid);
	}
	
	function	delete_application($applicationid)
	{
		$app = get_application_by_applicationid($applicationid);
		$host = get_host_by_hostid($app["hostid"]);

		// first delete child applications
		$db_applications = DBselect("select applicationid from applications where templateid=$applicationid");
		while($db_app = DBfetch($db_applications))
		{// recursion
			$result = delete_application($db_app["applicationid"]);
			if(!$result)	return	$result;
		}
 
		$result = DBexecute("delete from items_applications where applicationid=$applicationid");

		$result = DBexecute("delete from applications where applicationid=$applicationid");
		if($result)
		{
			info("Application '".$host["host"].":".$app["name"]."' deleted");
		}
		return $result;
	}

	function	get_application_by_applicationid($applicationid,$no_error_message=0)
	{
		$result = DBselect("select * from applications where applicationid=".$applicationid);
		$row=DBfetch($result);
		if($row)
		{
			return $row;
		}
		if($no_error_message == 0)
			error("No application with id=[$applicationid]");
		return	FALSE;
		
	}

	function	get_applications_by_templateid($applicationid)
	{
		return DBselect("select * from applications where templateid=".$applicationid);
	}

	function	get_realhost_by_applicationid($applicationid)
	{
		$application = get_application_by_applicationid($applicationid);
		if($application["templateid"] > 0)
			return get_realhost_by_applicationid($application["templateid"]);

		return get_host_by_applicationid($applicationid);
	}

	function	get_host_by_applicationid($applicationid)
	{
		$sql="select h.* from hosts h, applications a where a.hostid=h.hostid and a.applicationid=$applicationid";
		$result=DBselect($sql);
		$row=DBfetch($result);
		if($row)
		{
			return $row;
		}
		error("No host with applicationid=[$applicationid]");
		return	FALSE;
	}

	function	get_items_by_applicationid($applicationid)
	{
		return DBselect("select i.* from items i,items_applications ia where i.itemid=ia.itemid and ia.applicationid=$applicationid");
	}

	function	get_applications_by_hostid($hostid)
	{
		return DBselect("select * from applications where hostid=$hostid");
	}

	function        delete_template_applications($hostid, $templateid = null, $unlink_mode = false)
	{
		$db_apps = get_applications_by_hostid($hostid);
		while($db_app = DBfetch($db_apps))
		{
			if($db_app["templateid"] == 0)
				continue;

			if($templateid != null && $templateid != $db_app["templateid"])
				continue;
			
			if($unlink_mode)
			{
				if(DBexecute("update applications set templateid=0 where applicationid=".$db_app["applicationid"]))
				{
					info("Application '".$db_app["name"]."' unlinked");
				}
			}
			else
			{
				delete_application($db_app["applicationid"]);
			}
		}
	}

	function	copy_template_applications($hostid, $templateid = null, $copy_mode = false)
	{
		if(null == $templateid)
		{
			$host = get_host_by_hostid($hostid);
			$templateid = $host["templateid"];
		}

		$db_tmp_applications = get_applications_by_hostid($templateid);

		while($db_tmp_app = DBfetch($db_tmp_applications))
		{
			add_application(
				$db_tmp_app["name"],
				$hostid,
				$copy_mode ? 0 : $db_tmp_app["applicationid"]);
		}
	}
?>
