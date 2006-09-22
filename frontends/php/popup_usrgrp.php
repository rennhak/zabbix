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
	include "include/config.inc.php";
	include "include/forms.inc.php";

	$dstfrm		= get_request("dstfrm",		0);	// destination form
	$list_name	= get_request("list_name",	0);	// output field on destination form
	$var_name	= get_request("var_name",	0);	// second output field on destination form

	$page["title"] = "S_GROUPS";
	$page["file"] = "popup_usrgrp.php";
	show_header($page["title"],0,1);

	insert_confirm_javascript();
?>
<?php
	show_header2(S_GROUPS);
?>

<script language="JavaScript" type="text/javascript">
<!--
function add_var_to_opener_obj(obj,name,value)
{
        new_variable = window.opener.document.createElement('input');
        new_variable.type = 'hidden';
        new_variable.name = name;
        new_variable.value = value;

        obj.appendChild(new_variable);
}

function add_group(formname,group_id,name)
{
        var form = window.opener.document.forms[formname];

        if(!form)
        {
                window.close();
		return false;
        }

	add_var_to_opener_obj(form,'new_group[usrgrpid]',group_id);
	add_var_to_opener_obj(form,'new_group[name]',name);

	form.submit();
	window.close();
	return true;
}
-->
</script>


<?php
	$table = new CTableInfo(S_NO_GROUPS_DEFINED);
	$table->SetHeader(array(S_NAME));

	$result = DBselect("select * from usrgrp where ".id2nodeid('usrgrpid')."=$ZBX_CURNODEID order by name");
	while($row = DBfetch($result))
	{
		$name = new CLink($row["name"],"#","action");
		$name->SetAction('return add_group("'.$dstfrm.'",'.$row['usrgrpid'].',"'.$row['name'].'");');
		$table->AddRow($name);
	}
	$table->Show();

	show_page_footer(false);
?>
