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
	require_once 'include/config.inc.php';
	require_once 'include/services.inc.php';

	$page['file']	= 'chart5.php';
	$page['title']	= "S_CHART";
	$page['type']	= PAGE_TYPE_IMAGE;

include_once 'include/page_header.php';

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		'serviceid'=>		array(T_ZBX_INT, O_MAND,P_SYS,	DB_ID,		NULL)
	);

	check_fields($fields);
?>
<?php
	if(!DBfetch(DBselect('select serviceid from services where serviceid='.$_REQUEST['serviceid']))){
		fatal_error(S_NO_IT_SERVICE_DEFINED);
	}

	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, array(), PERM_RES_IDS_ARRAY);
	
	$sql = 'SELECT s.serviceid '.
				' FROM services s '.
				' WHERE (s.triggerid is NULL OR '.DBcondition('s.triggerid',$available_triggers).') '.
					' AND s.serviceid='.$_REQUEST['serviceid'];

	if(!$service = DBfetch(DBselect($sql,1))){
		access_deny();
	}
?>
<?php
	$start_time = time(NULL);

	$sizeX=900;
	$sizeY=300;

	$shiftX=12;
	$shiftYup=17;
	$shiftYdown=25+15*3;

	$im = imagecreate($sizeX+$shiftX+61,$sizeY+$shiftYup+$shiftYdown+10); 
  
	$red=imagecolorallocate($im,255,0,0); 
	$darkred=imagecolorallocate($im,150,0,0); 
	$green=imagecolorallocate($im,0,255,0); 
	$darkgreen=imagecolorallocate($im,0,150,0); 
	$blue=imagecolorallocate($im,0,0,255); 
	$darkblue=imagecolorallocate($im,0,0,150); 
	$yellow=imagecolorallocate($im,255,255,0); 
	$darkyellow=imagecolorallocate($im,150,150,0); 
	$cyan=imagecolorallocate($im,0,255,255); 
	$black=imagecolorallocate($im,0,0,0); 
	$gray=imagecolorallocate($im,150,150,150); 
	$white=imagecolorallocate($im,255,255,255); 
	$bg=imagecolorallocate($im,6+6*16,7+7*16,8+8*16);

	$x=imagesx($im); 
	$y=imagesy($im);
  
	imagefilledrectangle($im,0,0,$x,$y,$white);
	imagerectangle($im,0,0,$x-1,$y-1,$black);

	$str=$service['name'].' (year '.date('Y').')';
	$x=imagesx($im)/2-imagefontwidth(4)*strlen($str)/2;
	imagestring($im, 4,$x,1, $str , $darkred);

	$now = time(NULL);
	$to_time=$now;

	$count_now=array();
	$problem=array();

	$year=date('Y');
	$start=mktime(0,0,0,1,1,$year);

	$wday=date('w',$start);
	if($wday==0) $wday=7;
	$start=$start-($wday-1)*24*3600;

	for($i=0;$i<52;$i++){
		if(($period_start=$start+7*24*3600*$i) > time())
			break;
			
		if(($period_end=$start+7*24*3600*($i+1)) > time())
			$period_end = time();

		$stat = calculate_service_availability($_REQUEST['serviceid'],$period_start,$period_end);
		$problem[$i]=$stat['problem'];
		$ok[$i]=$stat['ok'];
		$count_now[$i]=1;
	}

	for($i=0;$i<=$sizeY;$i+=$sizeY/10){
		DashedLine($im,$shiftX,$i+$shiftYup,$sizeX+$shiftX,$i+$shiftYup,$gray);
	}

	for($i = 0, $period_start = $start; $i <= $sizeX; $i += $sizeX/52, $period_start += 7*24*3600){
		DashedLine($im,$i+$shiftX,$shiftYup,$i+$shiftX,$sizeY+$shiftYup,$gray);
		imagestringup($im, 1,$i+$shiftX-4, $sizeY+$shiftYup+32, date('d.M',$period_start) , $black);
	}

	$maxY = max(max($problem), 100);
	$minY = 0;

	$maxX = 900;
	$minX = 0;

	for($i=1;$i<=52;$i++){
		if(!isset($ok[$i-1])) continue;
		$x2=($sizeX/52)*($i-1-$minX)*$sizeX/($maxX-$minX);

		$y2=$sizeY*($ok[$i-1]-$minY)/($maxY-$minY);
		$y2=$sizeY-$y2;

		imagefilledrectangle($im,$x2+$shiftX,$y2+$shiftYup,$x2+$shiftX+8,$sizeY+$shiftYup,imagecolorallocate($im,120,235,120));
		imagerectangle($im,$x2+$shiftX,$y2+$shiftYup,$x2+$shiftX+8,$sizeY+$shiftYup,$black);

		imagefilledrectangle($im,$x2+$shiftX,$shiftYup,$x2+$shiftX+8,$y2+$shiftYup,imagecolorallocate($im,235,120,120));
		imagerectangle($im,$x2+$shiftX,$shiftYup,$x2+$shiftX+8,$y2+$shiftYup,$black);
	}

	for($i=0;$i<=$sizeY;$i+=$sizeY/10){
		imagestring($im, 1, $sizeX+5+$shiftX, $sizeY-$i-4+$shiftYup, ($i*($maxY-$minY)/$sizeY+$minY).'%' , imagecolorallocate($im,200,40,40));
	}

	imagefilledrectangle($im,$shiftX,$sizeY+$shiftYup+39+15*0,$shiftX+5,$sizeY+$shiftYup+35+9+15*0,imagecolorallocate($im,120,235,120));
	imagerectangle($im,$shiftX,$sizeY+$shiftYup+39+15*0,$shiftX+5,$sizeY+$shiftYup+35+9+15*0,$black);
	imagestring($im, 2,$shiftX+9,$sizeY+$shiftYup+15*0+35, 'OK (%)', $black);

	imagefilledrectangle($im,$shiftX,$sizeY+$shiftYup+39+15*1,$shiftX+5,$sizeY+$shiftYup+35+9+15*1,$darkred);
	imagerectangle($im,$shiftX,$sizeY+$shiftYup+39+15*1,$shiftX+5,$sizeY+$shiftYup+15+9+35*1,$black);
	imagestring($im, 2,$shiftX+9,$sizeY+$shiftYup+15*1+35, 'PROBLEMS (%)', $black);

	imagestringup($im,0,imagesx($im)-10,imagesy($im)-50, 'http://www.zabbix.com', $gray);

	$end_time=time(NULL);
	imagestring($im, 0,imagesx($im)-100,imagesy($im)-12,'Generated in '.($end_time-$start_time).' sec', $gray);

	ImageOut($im); 
	imagedestroy($im); 
?>
<?php

include_once 'include/page_footer.php';

?>