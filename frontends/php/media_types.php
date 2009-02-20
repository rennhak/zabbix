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
	require_once "include/media.inc.php";
	require_once "include/forms.inc.php";

	$page["title"] = "S_MEDIA_TYPES";
	$page["file"] = "media_types.php";
	$page['hist_arg'] = array('form','mediatypeid');

include_once "include/page_header.php";

?>
<?php
	$fields=array(
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION

// media form
		"mediatypeid"=>		array(T_ZBX_INT, O_NO,	P_SYS,	DB_ID,'(isset({form})&&({form}=="update"))'),
		"type"=>		array(T_ZBX_INT, O_OPT,	NULL,	IN(implode(',',array(MEDIA_TYPE_EMAIL,MEDIA_TYPE_EXEC,MEDIA_TYPE_SMS,MEDIA_TYPE_JABBER))),'(isset({save}))'),
		"description"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'(isset({save}))'),
		"smtp_server"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_EMAIL.')&&isset({save})'),
		"smtp_helo"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_EMAIL.')&&isset({save})'),
		"smtp_email"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_EMAIL.')&&isset({save})'),
		"exec_path"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_EXEC.')&&isset({save})'),
		"gsm_modem"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_SMS.')&&isset({save})'),
		"username"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'(isset({type})&&{type}=='.MEDIA_TYPE_JABBER.')&&isset({save})'),
		"password"=>		array(T_ZBX_STR, O_OPT,	NULL,	NOT_EMPTY,'isset({type})&&({type}=='.MEDIA_TYPE_JABBER.')&&isset({save})'),
/* actions */
		"save"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"delete"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"cancel"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
/* other */
		"form"=>		array(T_ZBX_STR, O_OPT, P_SYS,	NULL,	NULL),
		"form_refresh"=>	array(T_ZBX_INT, O_OPT,	NULL,	NULL,	NULL)
	);
	
	check_fields($fields);
	validate_sort_and_sortorder('mt.description',ZBX_SORT_UP);
?>
<?php

/* MEDIATYPE ACTIONS */
	$result = 0;
	if(isset($_REQUEST["save"])){
		if(isset($_REQUEST["mediatypeid"])){
/* UPDATE */
			$action = AUDIT_ACTION_UPDATE;
			$result=update_mediatype($_REQUEST["mediatypeid"],
				$_REQUEST["type"],$_REQUEST["description"],get_request("smtp_server"),
				get_request("smtp_helo"),get_request("smtp_email"),get_request("exec_path"),
				get_request("gsm_modem"),get_request('username'),get_request('password'));

			show_messages($result, S_MEDIA_TYPE_UPDATED, S_MEDIA_TYPE_WAS_NOT_UPDATED);
		}
		else{
/* ADD */
			$action = AUDIT_ACTION_ADD;
			$result=add_mediatype(
				$_REQUEST["type"],$_REQUEST["description"],get_request("smtp_server"),
				get_request("smtp_helo"),get_request("smtp_email"),get_request("exec_path"),
				get_request("gsm_modem"),get_request('username'),get_request('password'));

			show_messages($result, S_ADDED_NEW_MEDIA_TYPE, S_NEW_MEDIA_TYPE_WAS_NOT_ADDED);
		}
		if($result)
		{
			add_audit($action,AUDIT_RESOURCE_MEDIA_TYPE,
				"Media type [".$_REQUEST["description"]."]");

			unset($_REQUEST["form"]);
		}
	} elseif(isset($_REQUEST["delete"])&&isset($_REQUEST["mediatypeid"])) {
/* DELETE */
		$mediatype=get_mediatype_by_mediatypeid($_REQUEST["mediatypeid"]);
		$result=delete_mediatype($_REQUEST["mediatypeid"]);
		show_messages($result, S_MEDIA_TYPE_DELETED, S_MEDIA_TYPE_WAS_NOT_DELETED);
		if($result)
		{
			add_audit(AUDIT_ACTION_DELETE,AUDIT_RESOURCE_MEDIA_TYPE,
				"Media type [".$mediatype["description"]."]");

			unset($_REQUEST["form"]);
		}
	}

?>
<?php

	$form = new CForm();
	$form->SetMethod('get');
	
	$form->AddItem(new CButton("form",S_CREATE_MEDIA_TYPE));
	
	$row_count = 0;
	$numrows = new CSpan(null,'info');
	$numrows->addOption('name','numrows');	
	$header = get_table_header(array(S_CONFIGURATION_OF_MEDIA_TYPES_BIG,
					new CSpan(SPACE.SPACE.'|'.SPACE.SPACE, 'divider'),
					S_FOUND.': ',$numrows,)
					);			
	show_table_header($header, $form);

?>
<?php
	if(isset($_REQUEST["form"]))
	{
		echo SBR;
		insert_media_type_form();
	}
	else
	{
		$table=new CTableInfo(S_NO_MEDIA_TYPES_DEFINED);
		$table->setHeader(array(
			make_sorting_link(S_TYPE,'mt.type'),
			make_sorting_link(S_DESCRIPTION,'mt.description'),
			S_DETAILS
		));

		$result=DBselect('SELECT mt.* '.
					' FROM media_type mt'.
					' WHERE '.DBin_node('mt.mediatypeid').
					order_by('mt.type,mt.description'));
		while($row=DBfetch($result))
		{
			switch($row['type'])
			{
				case MEDIA_TYPE_EMAIL:
					$details =
						S_SMTP_SERVER.": '".$row['smtp_server']."', ".
						S_SMTP_HELO.": '".$row['smtp_helo']."', ". 
						S_SMTP_EMAIL.": '".$row['smtp_email']."'";
					break;
				case MEDIA_TYPE_EXEC:
					$details = S_SCRIPT_NAME.": '".$row['exec_path']."'";
					break;
				case MEDIA_TYPE_SMS:
					$details = S_GSM_MODEM.": '".$row['gsm_modem']."'";
					break;
				case MEDIA_TYPE_JABBER:
					$details = S_JABBER_IDENTIFIER.": '".$row['username']."'";
					break;
				default:
					$details = '';
			}
			
			$table->addRow(array(
				media_type2str($row['type']),
				new CLink($row["description"],"?&form=update&mediatypeid=".$row["mediatypeid"],'action'),
				$details));
			$row_count++;
		}
		$table->show();
	}
	zbx_add_post_js('insert_in_element("numrows","'.$row_count.'");');

?>

<?php

include_once "include/page_footer.php";

?>
