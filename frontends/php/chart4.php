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
	require_once "include/triggers.inc.php";

	$page["file"]	= "chart4.php";
	$page["title"]	= "S_CHART";
	$page["type"]	= PAGE_TYPE_IMAGE;

include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		'triggerid'=>		array(T_ZBX_INT, O_MAND,P_SYS,	DB_ID,		NULL)
	);

	check_fields($fields);
?>
<?php
	$sql = 'SELECT DISTINCT i.hostid '.
			' FROM functions f, items i '.
			' WHERE f.triggerid='.$_REQUEST['triggerid'].
				' AND i.itemid=f.itemid';
	if(!$host = DBfetch(DBselect($sql))){
		fatal_error(S_NO_TRIGGER_DEFINED);
	}
	
	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, array($host['hostid']));
				
	$sql = 'SELECT DISTINCT t.triggerid,t.description,t.expression, h.host,h.hostid '.
			' FROM hosts h, items i, functions f, triggers t'.
			' WHERE h.hostid=i.hostid '.
				' AND i.itemid=f.itemid '.
				' AND f.triggerid=t.triggerid '.
				' AND t.triggerid='.$_REQUEST['triggerid'].
				' AND '.DBcondition('t.triggerid',$available_triggers);
			
	if(!$db_data = DBfetch(DBselect($sql))){
		access_deny();
	}

	$start_time = time(NULL);

	$sizeX		= 900;
	$sizeY		= 300;

	$shiftX		= 12;
	$shiftYup	= 17;
	$shiftYdown	= 25+15*3;

	$im = imagecreate($sizeX+$shiftX+61,$sizeY+$shiftYup+$shiftYdown+10); 
	
	$red		= imagecolorallocate($im,255,0,0); 
	$darkred	= imagecolorallocate($im,150,0,0); 
	$green		= imagecolorallocate($im,0,255,0); 
	$darkgreen	= imagecolorallocate($im,0,150,0); 
	$bluei		= imagecolorallocate($im,0,0,255); 
	$darkblue	= imagecolorallocate($im,0,0,150); 
	$yellow		= imagecolorallocate($im,255,255,0); 
	$darkyellow	= imagecolorallocate($im,150,150,0); 
	$cyan		= imagecolorallocate($im,0,255,255); 
	$black		= imagecolorallocate($im,0,0,0); 
	$gray		= imagecolorallocate($im,150,150,150); 
	$white		= imagecolorallocate($im,255,255,255); 
	$bg			= imagecolorallocate($im,6+6*16,7+7*16,8+8*16);

	$x=imagesx($im); 
	$y=imagesy($im);
  
	imagefilledrectangle($im,0,0,$x,$y,$white);
	imagerectangle($im,0,0,$x-1,$y-1,$black);

	$str = expand_trigger_description_by_data($db_data);

	$str = $str." (year ".date("Y").")";
	$x = imagesx($im)/2-imagefontwidth(4)*strlen($str)/2;
	imagestring($im, 4,$x,1, $str , $darkred);

	$now = time(NULL);

	$count_now=array();
	$true = array();
	$false = array();
	$unknown = array();

	$year=date("Y");
	$start=mktime(0,0,0,1,1,$year);

	$wday=date("w",$start);
	if($wday==0) $wday=7;
	$start=$start-($wday-1)*24*3600;
	
	$weeks = (int)(date('z')/7 +1);

	for($i=0;$i<$weeks;$i++){
		$period_start=$start+7*24*3600*$i;
		$period_end=$start+7*24*3600*($i+1);
		
		$stat=calculate_availability($_REQUEST['triggerid'],$period_start,$period_end);
		$true[$i]=$stat['true'];
		$false[$i]=$stat['false'];
		$unknown[$i]=$stat['unknown'];
		$count_now[$i]=1;
//SDI($false[$i]);
	}

	for($i=0;$i<=$sizeY;$i+=$sizeY/10){
		DashedLine($im,$shiftX,$i+$shiftYup,$sizeX+$shiftX,$i+$shiftYup,$gray);
	}

	$j=0;
	for($i=0;$i<=$sizeX;$i+=$sizeX/52){
		DashedLine($im,$i+$shiftX,$shiftYup,$i+$shiftX,$sizeY+$shiftYup,$gray);
		$period_start=$start+7*24*3600*$j;
		imagestringup($im, 1,$i+$shiftX-4, $sizeY+$shiftYup+32, date("d.M",$period_start) , $black);
		$j++;
	}

	$maxY=100;
	$tmp=max($true);
	if($tmp>$maxY){
		$maxY=$tmp;
	}
	$minY=0;

	$maxX=900;
	$minX=0;

	for($i=0;$i<$weeks;$i++){
		$x1=(900/52)*$sizeX*($i-$minX)/($maxX-$minX);
		
//		imagefilledrectangle($im,$x1+$shiftX,$shiftYup,$x1+$shiftX+8,$sizeY+$shiftYup,imagecolorallocate($im,0,0,0)); 	// WHITE

		$yt=$sizeY*$true[$i]/100;
		if($yt > 0) imagefilledrectangle($im,$x1+$shiftX,$shiftYup,$x1+$shiftX+8,$yt+$shiftYup,imagecolorallocate($im,235,120,120));	// RED

		$yu=(int)($sizeY*$unknown[$i]/100+0.5);
		if($yu > 0) imagefilledrectangle($im,$x1+$shiftX,$yt+$shiftYup,$x1+$shiftX+8,$yt+$yu+$shiftYup,imagecolorallocate($im,235,235,235)); 	// UNKNOWN

		$yf=$sizeY*$false[$i]/100;
		if($yf > 0) imagefilledrectangle($im,$x1+$shiftX,$yt+$yu+$shiftYup,$x1+$shiftX+8,$sizeY+$shiftYup,imagecolorallocate($im,120,235,120));  // GREEN

//SDI($yt.'+'.$yf.'+'.$yu);
		if($yt+$yf+$yu > 0) imagerectangle($im,$x1+$shiftX,$shiftYup,$x1+$shiftX+8,$sizeY+$shiftYup,$black);
	}

	for($i=0;$i<=$sizeY;$i+=$sizeY/10){
		imagestring($im, 1, $sizeX+5+$shiftX, $sizeY-$i-4+$shiftYup, $i*($maxY-$minY)/$sizeY+$minY , $darkred);
	}

	imagefilledrectangle($im,$shiftX,$sizeY+$shiftYup+39+15*0,$shiftX+5,$sizeY+$shiftYup+35+9+15*0,imagecolorallocate($im,120,235,120));
	imagerectangle($im,$shiftX,$sizeY+$shiftYup+39+15*0,$shiftX+5,$sizeY+$shiftYup+35+9+15*0,$black);
	imagestring($im, 2,$shiftX+9,$sizeY+$shiftYup+15*0+35, "FALSE (%)", $black);

	imagefilledrectangle($im,$shiftX,$sizeY+$shiftYup+39+15*1,$shiftX+5,$sizeY+$shiftYup+35+9+15*1,imagecolorallocate($im,235,120,120));
	imagerectangle($im,$shiftX,$sizeY+$shiftYup+39+15*1,$shiftX+5,$sizeY+$shiftYup+15+9+35*1,$black);
	imagestring($im, 2,$shiftX+9,$sizeY+$shiftYup+15*1+35, "TRUE (%)", $black);

	imagefilledrectangle($im,$shiftX,$sizeY+$shiftYup+39+15*2,$shiftX+5,$sizeY+$shiftYup+35+9+15*2,imagecolorallocate($im,220,220,220));
	imagerectangle($im,$shiftX,$sizeY+$shiftYup+39+15*2,$shiftX+5,$sizeY+$shiftYup+35+9+15*2,$black);
	imagestring($im, 2,$shiftX+9,$sizeY+$shiftYup+15*2+35, "UNKNOWN (%)", $black);

	imagestringup($im,0,imagesx($im)-10,imagesy($im)-50, "http://www.zabbix.com", $gray);

	$end_time=time(NULL);
	imagestring($im, 0,imagesx($im)-100,imagesy($im)-12,"Generated in ".($end_time-$start_time)." sec", $gray);

	ImageOut($im); 
	imagedestroy($im); 
?>
<?php

include_once "include/page_footer.php";

?>
