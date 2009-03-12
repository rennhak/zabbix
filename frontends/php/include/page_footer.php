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
	require_once("include/config.inc.php");

	global $USER_DETAILS;
	global $page;
	global $ZBX_PAGE_POST_JS;

	if(!defined('PAGE_HEADER_LOADED')){
		define ('PAGE_HEADER_LOADED', 1);
	}
	
//------------------------------------- <HISTORY> ---------------------------------------
	add_user_history($page);
//------------------------------------- </HISTORY> --------------------------------------

	show_messages();
		
	if($page['type'] == PAGE_TYPE_HTML){
?>
<script language="JavaScript" type="text/javascript">
<!--
function zbxCallPostScripts(){
<?php
		if(isset($ZBX_PAGE_POST_JS)){
			foreach($ZBX_PAGE_POST_JS as $script){
				echo $script."\n";
			}
		}
?>
}

try{
	chkbx_range_ext.init();
}
catch(e){
	throw('Checkbox extension failed!');
}
-->
</script>
<?php
//SDI(SBR.'SELECTS: '.$DB['SELECT_COUNT'].SPACE.SPACE.'EXECUTE: '.$DB['EXECUTE_COUNT'].SPACE.SPACE.'TOTAL: '.($DB['EXECUTE_COUNT']+$DB['SELECT_COUNT']));
		if(!defined('ZBX_PAGE_NO_MENU') && !defined('ZBX_PAGE_NO_FOOTER')){
			$table = new CTable(NULL,"page_footer");
			$table->SetCellSpacing(0);
			$table->SetCellPadding(1);
			$table->AddRow(array(
				new CCol(new CLink(
					S_ZABBIX_VER.SPACE.S_COPYRIGHT_BY.SPACE.S_SIA_ZABBIX,
					"http://www.zabbix.com", "highlight", null, true),
					"page_footer_l"),
				new CCol(array(
						new CSpan(SPACE.SPACE."|".SPACE.SPACE,"divider"),
						new CSpan(($USER_DETAILS['userid'] == 0)?S_NOT_CONNECTED:S_CONNECTED_AS.SPACE."'".$USER_DETAILS["alias"]."'".
						(ZBX_DISTRIBUTED ? SPACE.S_FROM_SMALL.SPACE."'".$USER_DETAILS["node"]['name']."'" : ''),'footer_sign')
					),
					"page_footer_r")
				));
			$table->Show();
		}

COpt::profiling_stop("page");
COpt::profiling_stop("script");

		echo "</body>\n";
		echo "</html>\n";
	}
	exit;
?>
