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
	$page["title"] = "S_ALERT_HISTORY_SMALL";
	$page["file"] = "alerts.php";
	show_header($page["title"],1,0);
?>

<?php
	if(isset($_REQUEST["start"])&&isset($_REQUEST["do"])&&($_REQUEST["do"]=="<< Prev 100"))
	{
		$_REQUEST["start"]-=100;
	}
	if(isset($_REQUEST["do"])&&($_REQUEST["do"]=="Next 100 >>"))
	{
		if(isset($_REQUEST["start"]))
		{
			$_REQUEST["start"]+=100;
		}
		else
		{
			$_REQUEST["start"]=100;
		}
	}
	if(isset($_REQUEST["start"])&&($_REQUEST["start"]<=0))
	{
		unset($_REQUEST["start"]);
	}
?>

<?php
	update_profile("web.menu.view.last",$page["file"]);
?>

<?php
	$h1="&nbsp;".S_ALERT_HISTORY_BIG;

	$h2="";

	if(isset($_REQUEST["start"]))
	{
		$h2=$h2."<input class=\"biginput\" name=\"start\" type=hidden value=".$_REQUEST["start"]." size=8>";
  		$h2=$h2."<input class=\"button\" type=\"submit\" name=\"do\" value=\"<< Prev 100\">";
	}
	else
	{
  		$h2=$h2."<input class=\"button\" type=\"submit\" disabled name=\"do\" value=\"<< Prev 100\">";
	}
  	$h2=$h2."<input class=\"button\" type=\"submit\" name=\"do\" value=\"Next 100 >>\">";

	show_header2($h1,$h2,"<form name=\"form2\" method=\"get\" action=\"alerts.php\">","</form>");
?>


<FONT COLOR="#000000">
<?php
	$sql="select max(alertid) as max from alerts";
	$result=DBselect($sql);
	$row=DBfetch($result);
	$maxalertid=@iif(DBnum_rows($result)>0,$row["max"],0);

	if(!isset($_REQUEST["start"]))
	{
		$sql="select a.alertid,a.clock,mt.description,a.sendto,a.subject,a.message,a.status,a.retries,a.error from alerts a,media_type mt where mt.mediatypeid=a.mediatypeid and a.alertid>$maxalertid-200 order by a.clock desc limit 200";
	}
	else
	{
		$sql="select a.alertid,a.clock,mt.description,a.sendto,a.subject,a.message,a.status,a.retries,a.error from alerts a,media_type mt where mt.mediatypeid=a.mediatypeid and a.alertid>$maxalertid-200-".$_REQUEST["start"]." order by a.clock desc limit ".($_REQUEST["start"]+500);
	}
	$result=DBselect($sql);

	$table = new Ctable(S_NO_ALERTS);
	$table->setHeader(array(S_TIME, S_TYPE, S_STATUS, S_RECIPIENTS, S_SUBJECT, S_MESSAGE, S_ERROR));
	$col=0;
	$zzz=0;
	while($row=DBfetch($result))
	{
		$zzz++;	
		if(!check_anyright("Default permission","R"))
                {
			continue;
		}

		if($col>100)	break;

		$time=date("Y.M.d H:i:s",$row["clock"]);

		if($row["status"] == 1)
		{
			$status=array("value"=>S_SENT,"class"=>"off");
		}
		else
		{
			$status=array("value"=>S_NOT_SENT,"class"=>"on");
		}
		$sendto=htmlspecialchars($row["sendto"]);
		$subject="<pre>".htmlspecialchars($row["subject"])."</pre>";
		$message="<pre>".htmlspecialchars($row["message"])."</pre>";
		if($row["error"] == "")
		{
			$error=array("value"=>"&nbsp;","class"=>"off");
		}
		else
		{
			$error=array("value"=>$row["error"],"class"=>"on");
		}
		$table->addRow(array(
			$time,
			$row["description"],
			$status,
			$sendto,
			$subject,
			$message,
			$error));
	}
	$table->show();
?>

<?php
	show_footer();
?>
