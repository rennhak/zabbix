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
	require_once "db.inc.php";

	function	permission2str($group_permission)
	{
		$str_perm[PERM_READ_WRITE]	= S_READ_WRITE;
		$str_perm[PERM_READ_ONLY]	= S_READ_ONLY;
		$str_perm[PERM_DENY]		= S_DENY;

		if(isset($str_perm[$group_permission]))
			return $str_perm[$group_permission];

		return S_UNCNOWN;
	}

/*****************************************
	CHECK USER AUTHORISATION
*****************************************/

	function	check_authorisation()
	{
		global	$page;
		global	$PHP_AUTH_USER,$PHP_AUTH_PW;
		global	$USER_DETAILS;
		global	$_COOKIE;
		global	$_REQUEST;
		global	$ZBX_LOCALNODEID;

		$USER_DETAILS = NULL;
		
		if(isset($_COOKIE["sessionid"]))
		{
			$sessionid = $_COOKIE["sessionid"];
			if(!($USER_DETAILS = DBfetch(DBselect("select u.*,s.* from sessions s,users u".
				" where s.sessionid=".zbx_dbstr($sessionid)." and s.userid=u.userid".
				" and ((s.lastaccess+u.autologout>".time().") or (u.autologout=0))".
				" and ".DBid2nodeid('u.userid')." = ".$ZBX_LOCALNODEID))))
			{
				setcookie("sessionid",$sessionid,time()-3600);
				DBexecute("delete from sessions where sessionid=".zbx_dbstr($sessionid));
				unset($_COOKIE["sessionid"]);
				unset($sessionid);

				$incorrect_session = true;
			}
			else
			{
				setcookie("sessionid",$sessionid);
				DBexecute("update sessions set lastaccess=".time()." where sessionid=".zbx_dbstr($sessionid));
			}
		}
		
		if(!$USER_DETAILS)
		{
			if(!($USER_DETAILS = DBfetch(DBselect("select u.* from users u where u.alias='guest'".
				" and ".DBid2nodeid('u.userid')."=$ZBX_LOCALNODEID"))))
			{
				$missed_user_guest = true;
			}
		}

		if($USER_DETAILS)
		{
			$USER_DETAILS['node'] = DBfetch(DBselect('select * from nodes where nodeid='.id2nodeid($USER_DETAILS['userid'])));
			if(empty($USER_DETAILS['node']))
			{
				$USER_DETAILS['node']['name'] = '- uncnown -';
				$USER_DETAILS['node']['nodeid'] = $ZBX_LOCALNODEID;
			}
		}
		else
		{
			$USER_DETAILS = array(
				"alias"	=>"- unknown -",
				"userid"=>0,
				"lang"	=>"en_gb",
				"type"	=>"0",
				"node"	=>array(
					"name"	=>'- uncnown -',
					"nodeid"=>0));
		}
		
		if(isset($incorrect_session) || isset($missed_user_guest))
		{
			if(isset($incorrect_session))	$message = "Session was ended, please relogin!";
			else if(isset($missed_user_guest))		$message = "Database corrupted, missed default user 'guest'";
			
			if($page["file"]!="index.php")
			{
				Redirect("index.php?message=".addslashes($message));
				exit;
			}
			if(!isset($_REQUEST['message'])) $_REQUEST['message'] = $message;
		}
	}

/***********************************************
	GET ACCESSIBLE RESOURCES BY USERID
************************************************/
	function	perm_mode2comparator($perm_mode)
	{
		switch($perm_mode)
		{
			case PERM_MODE_NE:	$perm_mode = '!='; break;
			case PERM_MODE_EQ:	$perm_mode = '=='; break;
			case PERM_MODE_GT:	$perm_mode = '>'; break;
			case PERM_MODE_LT:	$perm_mode = '<'; break;
			case PERM_MODE_LE:	$perm_mode = '<='; break;
			case PERM_MODE_GE:
			default:		$perm_mode = '>='; break;
		}
		return $perm_mode;
	}

	function	get_accessible_hosts_by_userid($userid,$perm,$perm_mode=null,$perm_res=null,$nodeid=null,$hostid=null)
	{

		if(is_null($perm_res))		$perm_res	= PERM_RES_STRING_LINE;
		if($perm == PERM_READ_LIST)	$perm		= PERM_READ_ONLY;

		$result = array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$host_data'; break;
			default:			$resdata = '$host_data["hostid"]'; break;
		}

COpt::counter_up('perm_host['.$userid.','.$perm.','.$perm_mode.','.$perm_res.','.$nodeid.']');
COpt::counter_up('perm');

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' and n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' and n.nodeid in ('.$nodeid.') ';
	
		if(is_null($hostid))		$where_hostid = '';
		else if(is_array($hostid))	$where_hostid = ' and h.hostid in ('.implode(',', $hostid).') ';
		else 				$where_hostid = ' and h.hostid in ('.$hostid.') ';
	
		$db_hosts = DBselect('select distinct n.nodeid,n.name as node_name,h.hostid,h.host, min(r.permission) as permission '.
			' from nodes n, users_groups ug '.
			' left join rights r on r.groupid=ug.usrgrpid and r.type='.RESOURCE_TYPE_GROUP.' and ug.userid='.$userid.
			' right join groups g on r.id=g.groupid '.
			' left join hosts_groups hg on g.groupid=hg.groupid '.
			' right join hosts h on hg.hostid=h.hostid '.
			' where '.DBid2nodeid('h.hostid').'=n.nodeid '.$where_nodeid.$where_hostid.' group by h.hostid'.
			' order by n.name, g.name, h.host');


		while($host_data = DBfetch($db_hosts))
		{
			/* if no rights defined used node rights */
			if(is_null($host_data['permission']))
			{
				if(!isset($nodes))
				{
					$nodes = get_accessible_nodes_by_userid($userid,
						PERM_DENY,PERM_MODE_GE,PERM_RES_DATA_ARRAY);
				}
				$host_data['permission'] = $nodes[$host_data['nodeid']]['permission'];
			}

			if(eval('return ('.$host_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
				continue;

			$result[$host_data['hostid']] = eval('return '.$resdata.';');
		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}

	function	get_accessible_groups_by_userid($userid,$perm,$perm_mode=null,$perm_res=null,$nodeid=null)
	{
		global $ZBX_LOCALNODEID;

		if(is_null($perm_mode))		$perm_mode	= PERM_MODE_GE;
		if(is_null($perm_res))		$perm_res	= PERM_RES_STRING_LINE;

		$result = array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$group_data'; break;
			default:			$resdata = '$group_data["groupid"]'; break;
		}

COpt::counter_up('perm_group['.$userid.','.$perm.','.$perm_mode.','.$perm_res.','.$nodeid.']');
COpt::counter_up('perm');

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' and n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' and n.nodeid in ('.$nodeid.') ';

		/* if no rights defined used node rights */
		$db_groups = DBselect('select n.nodeid,n.name as node_name,hg.groupid,hg.name, min(r.permission) as permission '.
			' from  nodes n, users_groups g '.
			' left join rights r on r.groupid=g.usrgrpid and r.type='.RESOURCE_TYPE_GROUP.' and g.userid='.$userid.
			' right join groups hg on r.id=hg.groupid '.
			' where '.DBid2nodeid('hg.groupid').'=n.nodeid '.$where_nodeid.
			' group by hg.groupid, hg.name, g.userid order by n.name, hg.name');

		while($group_data = DBfetch($db_groups))
		{
			/* deny if no rights defined */
			if(is_null($group_data['permission']))
			{
				if(!isset($nodes))
				{
					$nodes = get_accessible_nodes_by_userid($userid,
						PERM_DENY,PERM_MODE_GE,PERM_RES_DATA_ARRAY);
				}
				$group_data['permission'] = $nodes[$group_data['nodeid']]['permission'];
			}

			if(eval('return ('.$group_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
				continue;

			$result[$group_data['groupid']] = eval('return '.$resdata.';');
		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}

	function	get_accessible_nodes_by_userid($userid,$perm,$perm_mode=null,$perm_res=null,$nodeid=null)
	{
		global $ZBX_LOCALNODEID;

		if(is_null($perm_mode)) $perm_mode=PERM_MODE_GE;
		if(is_null($perm_res))	$perm_res=PERM_RES_STRING_LINE;

		$result= array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$node_data'; break;
			default:			$resdata = '$node_data["nodeid"]'; break;
		}

COpt::counter_up('perm_nodes['.$userid.','.$perm.','.$perm_mode.','.$perm_res.','.$nodeid.']');
COpt::counter_up('perm');

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' where n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' where  n.nodeid in ('.$nodeid.') ';

		$db_nodes = DBselect('select n.nodeid,n.name,min(r.permission) as permission'.
			' from users_groups g left join rights r on r.groupid=g.usrgrpid and'.
			' r.type='.RESOURCE_TYPE_NODE.' and g.userid='.$userid.
			' right join nodes n on r.id=n.nodeid'.$where_nodeid.
			' group by n.nodeid');

		while($node_data = DBfetch($db_nodes))
		{

			/* deny if no rights defined (for local node read/write)*/
			if(is_null($node_data['permission']))
				$node_data['permission'] = 
					($node_data['nodeid'] == $ZBX_LOCALNODEID) ? PERM_READ_WRITE : PERM_DENY;

			/* special processing for PERM_READ_LIST*/
			if(PERM_DENY == $node_data['permission'] && PERM_READ_LIST == $perm)
			{
				$groups = get_accessible_groups_by_userid($userid,
					$perm, PERM_MODE_GE,PERM_RES_DATA_ARRAY,$node_data['nodeid']);
				if(count($groups) == 0)  continue;
			}
			else
			{
				if(eval('return ('.$node_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
					continue;
			}

			$result[$node_data["nodeid"]] = eval('return '.$resdata.';');
		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}

/***********************************************
	GET ACCESSIBLE RESOURCES BY RIGHTS
************************************************/
	/* NOTE: right structure is

		$rights[i]['type']	= type of resource
		$rights[i]['permission']= permission for resource
		$rights[i]['id']	= resource id
		
	*/

	function	get_accessible_hosts_by_rights($rights,$perm,$perm_mode=null,$perm_res=null,$nodeid=null)
	{
		if(is_null($perm_res))		$perm_res	= PERM_RES_STRING_LINE;
		if($perm == PERM_READ_LIST)	$perm		= PERM_READ_ONLY;

		$result = array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$host_data'; break;
			default:			$resdata = '$host_data["hostid"]'; break;
		}

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' and n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' and n.nodeid in ('.$nodeid.') ';

		$db_hosts = DBselect('select n.nodeid,n.name as node_name,hg.groupid,h.hostid,h.host '.
			' from nodes n, hosts h left join hosts_groups hg on hg.hostid=h.hostid '.
			' where n.nodeid='.DBid2nodeid('h.hostid').$where_nodeid.' order by n.name,h.host');

		$res_perm = array();
		foreach($rights as $right)
		{
			$res_perm[$right['type']][$right['id']] = $right['permission'];
		}

		$host_perm = array();

		while($host_data = DBfetch($db_hosts))
		{
			if(isset($host_data['groupid']) && isset($res_perm[RESOURCE_TYPE_GROUP][$host_data['groupid']]))
			{
				$host_perm[$host_data['hostid']][RESOURCE_TYPE_GROUP][$host_data['groupid']] =
					$res_perm[RESOURCE_TYPE_GROUP][$host_data['groupid']];
			}

			if(isset($res_perm[RESOURCE_TYPE_NODE][$host_data['nodeid']]))
			{
				$host_perm[$host_data['hostid']][RESOURCE_TYPE_NODE] = $res_perm[RESOURCE_TYPE_NODE][$host_data['nodeid']];
			}
			$host_perm[$host_data['hostid']]['data'] = $host_data;

		}

		foreach($host_perm as $hostid => $host_data)
		{
			$host_data = $host_data['data'];

			if(isset($host_perm[$hostid][RESOURCE_TYPE_GROUP]))
			{
				$host_data['permission'] = min($host_perm[$hostid][RESOURCE_TYPE_GROUP]);
			}
			else if(isset($host_perm[$hostid][RESOURCE_TYPE_NODE]))
			{
				$host_data['permission'] = $host_perm[$hostid][RESOURCE_TYPE_NODE];
			}
			else
			{
				if(!isset($node_data[$host_data['nodeid']]))
				{
					$node_data = get_accessible_nodes_by_rights($rights,
						PERM_DENY, PERM_MODE_GE, PERM_RES_DATA_ARRAY, $host_data['nodeid']);
				}
				$host_data['permission'] = $node_data[$host_data['nodeid']]['permission'];
			}
			
			if(eval('return ('.$host_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
				continue;

			$result[$host_data['hostid']] = eval('return '.$resdata.';');

		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}
	function	get_accessible_groups_by_rights($rights,$perm,$perm_mode=null,$perm_res=null,$nodeid=null)
	{
		if(is_null($perm_mode)) $perm_mode=PERM_MODE_GE;
		if(is_null($perm_res))	$perm_res=PERM_RES_STRING_LINE;

		$result= array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$group_data'; break;
			default:			$resdata = '$group_data["groupid"]'; break;
		}

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' and n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' and n.nodeid in ('.$nodeid.') ';

		$group_perm = array();
		foreach($rights as $right)
		{
			if($right['type'] != RESOURCE_TYPE_GROUP) continue;
			$group_perm[$right['id']] = $right['permission'];
		}

		$db_groups = DBselect('select n.nodeid,n.name as node_name, g.groupid,g.name, '.PERM_DENY.' as permission from groups g, nodes n '.
				' where '.DBid2nodeid('g.groupid').'=n.nodeid '.$where_nodeid.
				' order by n.name, g.name');

		while($group_data = DBfetch($db_groups))
		{
			if(isset($group_perm[$group_data['groupid']]))
			{
				$group_data['permission'] = $group_perm[$group_data['groupid']];
			}
			else
			{
				if(!isset($node_data[$group_data['nodeid']]))
				{
					$node_data = get_accessible_nodes_by_rights($rights,
						PERM_DENY, PERM_MODE_GE, PERM_RES_DATA_ARRAY, $group_data['nodeid']);
				}
				$group_data['permission'] = $node_data[$group_data['nodeid']]['permission'];
			}
					
			if(eval('return ('.$group_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
				continue;

			$result[$group_data["groupid"]] = eval('return '.$resdata.';');
		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}

	function	get_accessible_nodes_by_rights($rights,$perm,$perm_mode=null,$perm_res=null,$nodeid=null)
	{
		global $ZBX_LOCALNODEID;

		if(is_null($perm_mode)) $perm_mode=PERM_MODE_GE;
		if(is_null($perm_res))	$perm_res=PERM_RES_STRING_LINE;

		$result= array();

		switch($perm_res)
		{
			case PERM_RES_DATA_ARRAY:	$resdata = '$node_data'; break;
			default:			$resdata = '$node_data["nodeid"]'; break;
		}

		if(is_null($nodeid))		$where_nodeid = '';
		else if(is_array($nodeid))	$where_nodeid = ' where n.nodeid in ('.implode(',', $nodeid).') ';
		else 				$where_nodeid = ' where  n.nodeid in ('.$nodeid.') ';

		$node_perm = array();
		foreach($rights as $right)
		{
			if($right['type'] != RESOURCE_TYPE_NODE) continue;
			$node_perm[$right['id']] = $right['permission'];
		}

		$db_nodes = DBselect('select n.nodeid,n.name, '.PERM_DENY.' as permission from nodes n '.$where_nodeid.' order by n.name');

		while($node_data = DBfetch($db_nodes))
		{
			if(isset($node_perm[$node_data['nodeid']]))
				$node_data['permission'] = $node_perm[$node_data['nodeid']];
			elseif($node_data['nodeid'] == $ZBX_LOCALNODEID) /* for local node default permission is READ_WRITE */
				$node_data['permission'] = PERM_READ_WRITE;

			/* special processing for PERM_READ_LIST*/
			if(PERM_DENY == $node_data['permission'] && PERM_READ_LIST == $perm)
			{
				$groups = get_accessible_groups_by_rights($rights,
					$perm, PERM_MODE_GE, PERM_RES_DATA_ARRAY, $node_data['nodeid']);
				if(count($groups) == 0)  continue;
			}
			else
			{
				if(eval('return ('.$node_data["permission"].' '.perm_mode2comparator($perm_mode).' '.$perm.')? 0 : 1;'))
					continue;
			}

			$result[$node_data["nodeid"]] = eval('return '.$resdata.';');
		}

		if($perm_res == PERM_RES_STRING_LINE) 
		{
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}

		return $result;
	}

?>
