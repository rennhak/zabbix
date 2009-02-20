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
	require_once "include/config.inc.php";

	$page["title"] = "S_RESOURCE";
	$page["file"] = "popup_right.php";

	define('ZBX_PAGE_NO_MENU', 1);
	
include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"dstfrm"=>		array(T_ZBX_STR, O_MAND,P_SYS,	NOT_EMPTY,		NULL),
		"permission"=>	array(T_ZBX_INT, O_MAND,P_SYS,	IN(PERM_DENY.','.PERM_READ_ONLY.','.PERM_READ_WRITE),	NULL),
		'nodeid'=>		array(T_ZBX_INT, O_OPT, P_SYS,	DB_ID,	NULL),
	);

	check_fields($fields);

	$dstfrm		= get_request("dstfrm",		0);			// destination form
	$permission	= get_request("permission",	PERM_DENY);		// right
	$nodeid		= get_request('nodeid', 	get_profile('web.popup_right.nodeid.last',get_current_nodeid(false)));

	update_profile('web.popup_right.nodeid.last', $nodeid);
?>
<script language="JavaScript" type="text/javascript">
<!--

function add_var_to_opener_obj(obj,name,value){
	new_variable = window.opener.document.createElement('input');
	new_variable.type = 'hidden';
	new_variable.name = name;
	new_variable.value = value;
	
	obj.appendChild(new_variable);
}

function add_right(formname,id,permission,name){
	var form = window.opener.document.forms[formname];
	
	if(!form){
		close_window();
	return false;
	}

	add_var_to_opener_obj(form,'new_right[id]',id);
	add_var_to_opener_obj(form,'new_right[permission]',permission);
	add_var_to_opener_obj(form,'new_right[name]',name);

	form.submit();
	close_window();
	return true;
}
-->
</script>
<?php
	$frmTitle = new CForm();
	$frmTitle->AddVar('dstfrm',$dstfrm);
	$frmTitle->AddVar('permission', $permission);

	if(ZBX_DISTRIBUTED){
		$available_nodes = get_accessible_nodes_by_user($USER_DETAILS, PERM_READ_WRITE,PERM_RES_IDS_ARRAY);

		$cmbResourceNode = new CComboBox('nodeid',$nodeid,'submit();');
		$cmbResourceNode->AddItem(0, S_ALL_S);
		
		$sql = 'SELECT name,nodeid FROM nodes WHERE '.DBcondition('nodeid',$available_nodes);
		$db_nodes = DBselect($sql);
		while($node = DBfetch($db_nodes)){
			$cmbResourceNode->AddItem($node['nodeid'], $node['name']);
		}
		
		$frmTitle->AddItem(array(S_NODE, SPACE, $cmbResourceNode));
	}

	show_table_header(permission2str($permission),$frmTitle);

	$table = new CTableInfo(S_NO_RESOURCES_DEFINED);
	$table->SetHeader(array(S_NAME));
	
	$db_resources = null;

	$db_resources = DBselect('SELECT n.name as node_name, g.name as name, g.groupid as id'.
					' FROM groups g '.
						' LEFT JOIN nodes n on '.DBid2nodeid('g.groupid').'=n.nodeid '.
					($nodeid?' WHERE nodeid='.$nodeid:'').
					' ORDER BY n.name, g.name');

	while($db_resource = DBfetch($db_resources)){
		if(isset($db_resource['node_name']))
			$db_resource['name'] = $db_resource['node_name'].':'.$db_resource['name'];

		$name = new CSpan($db_resource['name'],'link');
		$name->addAction('onclick', "return add_right('".$dstfrm."','".$db_resource['id']."','".$permission."','".$db_resource['name']."');");

		$table->AddRow(array($name));
	}

	$table->Show();
?>
<?php

include_once "include/page_footer.php";

?>
