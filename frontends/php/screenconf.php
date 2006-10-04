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
	require_once "include/screens.inc.php";
	require_once "include/forms.inc.php";

	$page["title"] = "S_SCREENS";
	$page["file"] = "screenconf.php";
	show_header($page["title"],0,0);
	insert_confirm_javascript();
?>
<?php
	if(isset($_REQUEST["save"])){
		if(isset($_REQUEST["screenid"]))
		{
			$result=update_screen($_REQUEST["screenid"],
				$_REQUEST["name"],$_REQUEST["hsize"],$_REQUEST["vsize"]);
			show_messages($result, S_SCREEN_UPDATED, S_CANNOT_UPDATE_SCREEN);
		} else {
			$result=add_screen($_REQUEST["name"],$_REQUEST["hsize"],$_REQUEST["vsize"]);
			show_messages($result,S_SCREEN_ADDED,S_CANNOT_ADD_SCREEN);
		}
		if($result){
			unset($_REQUEST["form"]);
			unset($_REQUEST["screenid"]);
		}
	}
	if(isset($_REQUEST["delete"])&&isset($_REQUEST["screenid"]))
	{
		$result=delete_screen($_REQUEST["screenid"]);
		show_messages($result, S_SCREEN_DELETED, S_CANNOT_DELETE_SCREEN);
		unset($_REQUEST["screenid"]);
	}
?>

<?php
	$form = new CForm("screenconf.php");
	$form->AddItem(new CButton("form",S_CREATE_SCREEN));
	show_header2(S_CONFIGURATION_OF_SCREENS_BIG, $form);
	echo BR;

	if(isset($_REQUEST["form"]))
	{
		insert_screen_form();
	}
	else
	{
		show_header2(S_SCREENS_BIG);

		$table = new CTableInfo(S_NO_SCREENS_DEFINED);
		$table->setHeader(array(S_NAME,S_DIMENSION_COLS_ROWS,S_SCREEN));

		$result=DBselect("select screenid,name,hsize,vsize from screens where ".DBid2nodeid("screenid")."=".$ZBX_CURNODEID.
				" order by name");
		while($row=DBfetch($result))
		{
//			if(!check_right("Screen","R",$row["screenid"]))		continue; /* TODO */

			$table->addRow(array(
				new CLink($row["name"],"screenconf.php?form=update&screenid=".$row["screenid"],
					'action'),
				$row["hsize"]." x ".$row["vsize"],
				new CLink(S_EDIT,"screenedit.php?screenid=".$row["screenid"])
				));
		}
		$table->show();
	}
?>

<?php
	show_page_footer();
?>
