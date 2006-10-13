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
	global $USER_DETAILS;
	global $page;

	if($page['type'] == PAGE_TYPE_HTML)
	{
		show_messages();

		if(!defined('ZBX_PAGE_NO_MENU') && !defined('ZBX_PAGE_NO_FOOTER'))
		{
			$table = new CTable(NULL,"page_footer");
			$table->SetCellSpacing(0);
			$table->SetCellPadding(1);
			$table->AddRow(array(
				new CCol(new CLink(
					S_ZABBIX_VER.SPACE.S_COPYRIGHT_BY.SPACE.S_SIA_ZABBIX,
					"http://www.zabbix.com", "highlight"),
					"page_footer_l"),
				new CCol(array(
						new CSpan(SPACE.SPACE."|".SPACE.SPACE,"divider"),
						S_CONNECTED_AS.SPACE."'".$USER_DETAILS["alias"]."'".SPACE.
						S_FROM_SMALL.SPACE."'".$USER_DETAILS["node"]['name']."'"
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
