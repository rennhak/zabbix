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

function SDI($msg="SDI") { echo "DEBUG INFO: "; if(is_array($msg)) print_r($msg); else echo($msg); echo BR; } // DEBUG INFO!!!


?>
<?php
	require_once 	"include/html.inc.php";
	require_once	"include/copt.lib.php";

// GLOBALS
	$USER_DETAILS	= array();
	$USER_RIGHTS	= array();
	$ERROR_MSG	= array();
	$INFO_MSG	= array();

	
	$ZBX_LOCALNODEID = 1; // Local node
// END OF GLOBALS

// if magic quotes on then get rid of them
	if (get_magic_quotes_gpc()) {
		$_GET    = zbx_stripslashes($_GET);
		$_POST	 = zbx_stripslashes($_POST);
		$_COOKIE = zbx_stripslashes($_COOKIE);
		$_REQUEST= zbx_stripslashes($_REQUEST);
	}

	require_once 	"include/defines.inc.php";
	require_once 	"include/db.inc.php";
	require_once 	"include/locales.inc.php";
	require_once 	"include/perm.inc.php";
	require_once 	"include/audit.inc.php";

// Include Validation

	require_once 	"include/validate.inc.php";

// Include Classes
	require_once("include/classes/ctag.inc.php");
	require_once("include/classes/cvar.inc.php");
	require_once("include/classes/cspan.inc.php");
	require_once("include/classes/cimg.inc.php");
	require_once("include/classes/clink.inc.php");
	require_once("include/classes/chelp.inc.php");
	require_once("include/classes/cbutton.inc.php");
	require_once("include/classes/clist.inc.php");
	require_once("include/classes/ccombobox.inc.php");
	require_once("include/classes/ctable.inc.php");
	require_once("include/classes/ctableinfo.inc.php");
	require_once("include/classes/ctextarea.inc.php");
	require_once("include/classes/ctextbox.inc.php");
	require_once("include/classes/cform.inc.php");
	require_once("include/classes/cfile.inc.php");
	require_once("include/classes/ccheckbox.inc.php");
	require_once("include/classes/cform.inc.php");
	require_once("include/classes/cformtable.inc.php");
	require_once("include/classes/cmap.inc.php");
	require_once("include/classes/cflash.inc.php");
	require_once("include/classes/ciframe.inc.php");
	require_once("include/classes/graph.inc.php");

// Include Tactical Overview modules

	include_once("include/classes/chostsinfo.mod.php");
	include_once("include/classes/ctriggerinfo.mod.php");
	include_once("include/classes/cserverinfo.mod.php");
	include_once("include/classes/cflashclock.mod.php");

	function	access_deny()
	{
		include_once "include/page_header.php";

		show_error_message(S_NO_PERMISSIONS);
		
		include_once "include/page_footer.php";
	}

	function zbx_stripslashes($value){
		if(is_array($value)){
			foreach($value as $id => $data)
				$value[$id] = zbx_stripslashes($data); 
				// $value = array_map('zbx_stripslashes',$value); /* don't use 'array_map' it buggy with indexes */
		} elseif (is_string($value)){
			$value = stripslashes($value);
		}
		return $value;
	}

	function get_request($name, $def){
		global  $_REQUEST;
		if(isset($_REQUEST[$name]))
			return $_REQUEST[$name];
		else
			return $def;
	}

	function info($msg)
	{
		global $INFO_MSG;

		if(is_array($INFO_MSG))
		{
			array_push($INFO_MSG,$msg);
		}
		else
		{
			$INFO_MSG=array($msg);
		}
	}

	function error($msg)
	{
		global $ERROR_MSG;
		if(is_array($ERROR_MSG))
		{
			array_push($ERROR_MSG,$msg);
		}
		else
		{
			$ERROR_MSG=array($msg);
		}
	}

	function fatal_error($msg)
	{
		error($msg);
		include_once "include/page_footer.php";
	}

	function getmicrotime()
	{
		list($usec, $sec) = explode(" ",microtime()); 
		return ((float)$usec + (float)$sec); 
	} 

	function	convert_units($value,$units)
	{
// Special processing for unix timestamps
		if($units=="unixtime")
		{
			$ret=date("Y.m.d H:i:s",$value);
			return $ret;
		}
// Special processing for seconds
		if($units=="s")
		{
			$ret="";

			$t=floor($value/(365*24*3600));
			if($t>0)
			{
				$ret=$t."y";
				$value=$value-$t*(365*24*3600);
			}
			$t=floor($value/(30*24*3600));
			if($t>0)
			{
				$ret=$ret.$t."m";
				$value=$value-$t*(30*24*3600);
			}
			$t=floor($value/(24*3600));
			if($t>0)
			{
				$ret=$ret.$t."d";
				$value=$value-$t*(24*3600);
			}
			$t=floor($value/(3600));
			if($t>0)
			{
				$ret=$ret.$t."h";
				$value=$value-$t*(3600);
			}
			$t=floor($value/(60));
			if($t>0)
			{
				$ret=$ret.$t."m";
				$value=$value-$t*(60);
			}
			$ret=$ret.$value."s";
		
			return $ret;	
		}

		$u="";

// Special processing for bits (kilo=1000, not 1024 for bits)
		if( ($units=="b") || ($units=="bps"))
		{
			$abs=abs($value);

			if($abs<1000)
			{
				$u="";
			}
			else if($abs<1000*1000)
			{
				$u="K";
				$value=$value/1000;
			}
			else if($abs<1000*1000*1000)
			{
				$u="M";
				$value=$value/(1000*1000);
			}
			else
			{
				$u="G";
				$value=$value/(1000*1000*1000);
			}
	
			if(round($value)==$value)
			{
				$s=sprintf("%.0f",$value);
			}
			else
			{
				$s=sprintf("%.2f",$value);
			}

			return "$s $u$units";
		}


		if($units=="")
		{
			if(round($value)==$value)
			{
				return sprintf("%.0f",$value);
			}
			else
			{
				return sprintf("%.2f",$value);
			}
		}

		$abs=abs($value);

		if($abs<1024)
		{
			$u="";
		}
		else if($abs<1024*1024)
		{
			$u="K";
			$value=$value/1024;
		}
		else if($abs<1024*1024*1024)
		{
			$u="M";
			$value=$value/(1024*1024);
		}
		else if($abs<1024*1024*1024*1024)
		{
			$u="G";
			$value=$value/(1024*1024*1024);
		}
		else
		{
			$u="T";
			$value=$value/(1024*1024*1024*1024);
		}

		if(round($value)==$value)
		{
			$s=sprintf("%.0f",$value);
		}
		else
		{
			$s=sprintf("%.2f",$value);
		}

		return "$s $u$units";
	}

	function	play_sound($filename)
	{
?>
<SCRIPT TYPE="text/javascript">
<!-- 
if (navigator.appName != "Microsoft Internet Explorer")
	document.writeln('<EMBED SRC="<?php echo $filename; ?>" AUTOSTART=TRUE WIDTH=0 HEIGHT=0 LOOP=0><P/>');
else
	document.writeln('<BGSOUND SRC="<?php echo $filename; ?>" LOOP=0/>');
// -->
</SCRIPT>
<NOSCRIPT>
	<BGSOUND SRC="<?php echo $filename; ?>"/>
</NOSCRIPT>
<?php
	}

//	The hash has form <md5sum of triggerid>,<sum of priorities>
	function	calc_trigger_hash()
	{

		$priority = array(0=>0, 1=>0, 2=>0, 3=>0, 4=>0, 5=>0);
		$triggerids="";

	       	$result=DBselect('select t.triggerid,t.priority from triggers t,hosts h,items i,functions f'.
			'  where t.value=1 and f.itemid=i.itemid and h.hostid=i.hostid and t.triggerid=f.triggerid and i.status=0');

		while($row=DBfetch($result))
		{
			$ack = get_last_event_by_triggerid($row["triggerid"]);
			if($ack["acknowledged"] == 1) continue;

			$triggerids="$triggerids,".$row["triggerid"];
			$priority[$row["priority"]]++;
		}

		$md5sum=md5($triggerids);

		$priorities=0;
		for($i=0;$i<=5;$i++)	$priorities += pow(100,$i)*$priority[$i];

		return	"$priorities,$md5sum";
	}

	function	select_config()
	{
		$row=DBfetch(DBselect("select * from config"));
		if($row)
		{
			return	$row;
		}
		else
		{
			error("Unable to select configuration");
		}
		return	$row;
	}

	function	show_messages($bool=TRUE,$okmsg=NULL,$errmsg=NULL)
	{
		global	$ERROR_MSG;
		global	$INFO_MSG;
		global	$page;

		if(!isset($page["type"])) $page["type"] = PAGE_TYPE_HTML;

		$message = array();
		$width = 0;
		$height= 0;

		if(!$bool && !is_null($errmsg))		$msg="ERROR: ".$errmsg;
		else if($bool && !is_null($okmsg))	$msg=$okmsg;

		if(isset($msg))
		{
			switch($page["type"])
			{
				case PAGE_TYPE_IMAGE:
					array_push($message, array(
						'text'	=> $msg,
						'color'	=> (!$bool) ? array('R'=>255,'G'=>0,'B'=>0) : array('R'=>34,'G'=>51,'B'=>68),
						'font'	=> 4));
					$width = max($width, ImageFontWidth(4) * strlen($msg) + 1);
					$height += imagefontheight(4) + 1;
					break;			
				case PAGE_TYPE_HTML:
				default:
					echo "<p align=center>";
					echo "<font color='".((!$bool) ? "#AA0000" : "#223344")."'>";
					echo "<b>[".htmlspecialchars($msg)."]</b>";
					echo "</font>";
					echo "</p>";
					break;
			}
		}

		if(is_array($INFO_MSG) && count($INFO_MSG)>0)
		{
			switch($page["type"])
			{
				case PAGE_TYPE_IMAGE:
					while($val = array_shift($INFO_MSG))
					{
						array_push($message, array(
							'text'	=> $val,
							'color'	=> array('R'=>155,'G'=>155,'B'=>55),
							'font'	=> 2));
						$width = max($width, ImageFontWidth(2) * strlen($val) + 1);
						$height += imagefontheight(2) + 1;
					}
					break;			
				case PAGE_TYPE_HTML:
					echo "<p align=center class=\"info\">";
					while($val = array_shift($INFO_MSG))
					{
						echo htmlspecialchars($val).BR;
					}
					echo "</p>";
					break;
			}
		}

		if(is_array($ERROR_MSG) && count($ERROR_MSG)>0)
		{
			switch($page["type"])
			{
				case PAGE_TYPE_IMAGE:
					while($val = array_shift($ERROR_MSG))
					{
						array_push($message, array(
							'text'	=> $val,
							'color'	=> array('R'=>255,'G'=>55,'B'=>55),
							'font'	=> 2));
						$width = max($width, ImageFontWidth(2) * strlen($val) + 1);
						$height += imagefontheight(2) + 1;
					}
					break;			
				case PAGE_TYPE_HTML:
					echo "<p align=center class=\"error\">";
					while($val = array_shift($ERROR_MSG))
					{
						echo htmlspecialchars($val).BR;
					}
					echo "</p>";
					break;
			}
		}

		if($page["type"] == PAGE_TYPE_IMAGE && count($message) > 0)
		{
			$width += 2;
			$height += 2;
			$canvas = imagecreate($width, $height);
			ImageFilledRectangle($canvas,0,0,$width,$height, ImageColorAllocate($canvas, 255, 255, 255));

			foreach($message as $id => $msg)
			{
				$message[$id]['y'] = 1 + (isset($previd) ? $message[$previd]['y'] + $message[$previd]['h'] : 0 );
				$message[$id]['h'] = imagefontheight($msg['font']);
				
				ImageString(
					$canvas,
					$msg['font'],
					1,
					$message[$id]['y'],
					$msg['text'],
					ImageColorAllocate($canvas, $msg['color']['R'], $msg['color']['G'], $msg['color']['B'])
					);
				
				$previd = $id;
			}
			ImageOut($canvas);
			ImageDestroy($canvas);
		}
	}

	function	show_message($msg)
	{
		show_messages(TRUE,$msg,'');
	}

	function	show_error_message($msg)
	{
		show_messages(FALSE,'',$msg);
	}

	function	parse_period($str)
	{
		$out = NULL;
		$str = trim($str,';');
		$periods = split(';',$str);
		foreach($periods as $preiod)
		{
			if(!ereg('^([1-7])-([1-7]),([0-9]{1,2}):([0-9]{1,2})-([0-9]{1,2}):([0-9]{1,2})$', $preiod, $arr))
				return NULL;
			for($i = $arr[1]; $i <= $arr[2]; $i++)
			{
				if(!isset($out[$i])) $out[$i] = array();
				array_push($out[$i],
					array(
						'start_h'	=> $arr[3],
						'start_m'	=> $arr[4],
						'end_h'		=> $arr[5],
						'end_m'		=> $arr[6]
					));
			}
		}
		return $out;
	}

	function	find_period_start($periods,$time)
	{
		$date = getdate($time);
		$wday = $date['wday'] == 0 ? 7 : $date['wday'];
		$curr = $date['hours']*100+$date['minutes'];

		if(isset($periods[$wday]))
		{
			$next_h = -1;
			$next_m = -1;
			foreach($periods[$wday] as $period)
			{
				$per_start = $period['start_h']*100+$period['start_m'];
				if($per_start > $curr)
				{
					if(($next_h == -1 && $next_m == -1) || ($per_start < ($next_h*100 + $next_m)))
					{
						$next_h = $period['start_h'];
						$next_m = $period['start_m'];
					}
					continue;
				}
				$per_end = $period['end_h']*100+$period['end_m'];
				if($per_end <= $curr) continue;
				return $time;
			}
			if($next_h >= 0 && $next_m >= 0)
			{
				return mktime($next_h, $next_m, 0, $date['mon'], $date['mday'], $date['year']);
			}
		}
		for($days=1; $days < 7 ; ++$days)
		{
			$new_wday = (($wday + $days - 1)%7 + 1);
			if(isset($periods[$new_wday ]))
			{
				$next_h = -1;
				$next_m = -1;
				foreach($periods[$new_wday] as $period)
				{
					$per_start = $period['start_h']*100+$period['start_m'];
					if(($next_h == -1 && $next_m == -1) || ($per_start < ($next_h*100 + $next_m)))
					{
						$next_h = $period['start_h'];
						$next_m = $period['start_m'];
					}
				}
				if($next_h >= 0 && $next_m >= 0)
				{
					return mktime($next_h, $next_m, 0, $date['mon'], $date['mday'] + $days, $date['year']);
				}
			}
		}
		return -1;
	}

	function	find_period_end($periods,$time,$max_time)
	{
		$date = getdate($time);
		$wday = $date['wday'] == 0 ? 7 : $date['wday'];
		$curr = $date['hours']*100+$date['minutes'];
//SDI("find_end: ".date('r',$time));
		if(isset($periods[$wday]))
		{
			$next_h = -1;
			$next_m = -1;
			foreach($periods[$wday] as $period)
			{
				$per_start = $period['start_h']*100+$period['start_m'];
				$per_end = $period['end_h']*100+$period['end_m'];
				if($per_start > $curr) continue;
				if($per_end < $curr) continue;

				if(($next_h == -1 && $next_m == -1) || ($per_end > ($next_h*100 + $next_m)))
				{
					$next_h = $period['end_h'];
					$next_m = $period['end_m'];
				}
			}
			if($next_h >= 0 && $next_m >= 0)
			{
				$new_time = mktime($next_h, $next_m, 0, $date['mon'], $date['mday'], $date['year']);

				if($new_time == $time)
					return $time;
				if($new_time > $max_time)
					return $max_time;

				$next_time = find_period_end($periods,$new_time,$max_time);
				if($next_time < 0)
					return $new_time;
				else
					return $next_time;
			}
		}
		return -1;
	}

	function	validate_period(&$str)
	{
		$str = trim($str,';');
		$out = "";
		$periods = split(';',$str);
		foreach($periods as $preiod)
		{
			// arr[idx]   1       2         3             4            5            6
			if(!ereg('^([1-7])-([1-7]),([0-9]{1,2}):([0-9]{1,2})-([0-9]{1,2}):([0-9]{1,2})$', $preiod, $arr))
				return -1;

			if($arr[1] > $arr[2]) // check week day
				return -1;
			if($arr[3] > 23 || $arr[3] < 0 || $arr[5] > 24 || $arr[5] < 0) // check hour
				return -1;
			if($arr[4] > 59 || $arr[4] < 0 || $arr[6] > 59 || $arr[6] < 0) // check min
				return -1;
			if(($arr[5]*100 + $arr[6]) > 2400) // check max time 24:00
				return -1;
			if(($arr[3] * 100 + $arr[4]) >= ($arr[5] * 100 + $arr[6])) // check time period
				return -1;

			$out .= sprintf("%d-%d,%02d:%02d-%02d:%02d",$arr[1],$arr[2],$arr[3],$arr[4],$arr[5],$arr[6]).';';
		}
		$str = $out;
//parse_period($str);
		return 0;
	}

	function	validate_float($str)
	{
//		echo "Validating float:$str<br>";
		if (eregi('^[ ]*([0-9]+)((\.)?)([0-9]*[KMG]{0,1})[ ]*$', $str, $arr)) 
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}

// Check if str has format #<float> or <float>
	function	validate_ticks($str)
	{
//		echo "Validating float:$str<br>";
		if (eregi('^[ ]*#([0-9]+)((\.)?)([0-9]*)[ ]*$', $str, $arr)) 
		{
			return 0;
		}
		else return validate_float($str);
	}

// Does expression match server:key.function(param) ?
	function	validate_simple_expression($expression)
	{
//		echo "Validating simple:$expression<br>";
// Before str()
// 		if (eregi('^\{([0-9a-zA-Z[.-.]\_\.]+)\:([]\[0-9a-zA-Z\_\/\.\,]+)\.((diff)|(min)|(max)|(last)|(prev))\(([0-9\.]+)\)\}$', $expression, $arr)) 
//		if (eregi('^\{([0-9a-zA-Z[.-.]\_\.]+)\:([]\[0-9a-zA-Z\_\/\.\,]+)\.((diff)|(min)|(max)|(last)|(prev)|(str))\(([0-9a-zA-Z\.\_\/\,]+)\)\}$', $expression, $arr)) 
// 		if (eregi('^\{([0-9a-zA-Z\_\.-]+)\:([]\[0-9a-zA-Z\_\*\/\.\,\:\(\) -]+)\.([a-z]{3,11})\(([#0-9a-zA-Z\_\/\.\,]+)\)\}$', $expression, $arr)) 
		if (eregi('^\{([0-9a-zA-Z\_\.-]+)\:([]\[0-9a-zA-Z\_\*\/\.\,\:\(\)\+ -]+)\.([a-z]{3,11})\(([#0-9a-zA-Z\_\/\.\,[:space:]]+)\)\}$', $expression, $arr))
		{
			$host=$arr[1];
			$key=$arr[2];
			$function=$arr[3];
			$parameter=$arr[4];

//SDI($host);
//SDI($key);
//SDI($function);
//SDI($parameter);

			$sql="select count(*) as cnt from hosts h,items i where h.host=".zbx_dbstr($host).
				" and i.key_=".zbx_dbstr($key)." and h.hostid=i.hostid";
//SDI($sql);
			$row=DBfetch(DBselect($sql));
			if($row["cnt"]!=1)
			{
				error("No such host ($host) or monitored parameter ($key)");
				return -1;
			}

			if(	($function!="last")&&
				($function!="diff")&&
				($function!="min") &&
				($function!="max") &&
				($function!="avg") &&
				($function!="sum") &&
				($function!="count") &&
				($function!="prev")&&
				($function!="delta")&&
				($function!="change")&&
				($function!="abschange")&&
				($function!="nodata")&&
				($function!="time")&&
				($function!="dayofweek")&&
				($function!="date")&&
				($function!="now")&&
				($function!="str")&&
				($function!="fuzzytime")&&
				($function!="logseverity")&&
				($function!="logsource")&&
				($function!="regexp")
			)
			{
				error("Unknown function [$function]");
				return -1;
			}


			if(in_array($function,array("last","diff","count",
						"prev","change","abschange","nodata","time","dayofweek",
						"date","now","fuzzytime"))
				&& (validate_float($parameter)!=0) )
			{
				error("[$parameter] is not a float");
				return -1;
			}

			if(in_array($function,array("min","max","avg","sum",
						"delta"))
				&& (validate_ticks($parameter)!=0) )
			{
				error("[$parameter] is not a float");
				return -1;
			}
		}
		else
		{
			error("Expression [$expression] does not match to [server:key.func(param)]");
			return -1;
		}
		return 0;
	}

	# Show screen cell containing plain text values
	function&	get_screen_plaintext($itemid,$elements)
	{
		global $DB_TYPE;

		$item=get_item_by_itemid($itemid);
		switch($item["value_type"])
		{
			case ITEM_VALUE_TYPE_FLOAT:	$history_table = "history";		break;
			case ITEM_VALUE_TYPE_UINT64:	$history_table = "history_uint";	break;
			case ITEM_VALUE_TYPE_TEXT:	$history_table = "history_text";	break;
			default:			$history_table = "history_str";		break;
		}

		$sql="select h.clock,h.value,i.valuemapid from ".$history_table." h, items i where".
			" h.itemid=i.itemid and i.itemid=$itemid order by clock desc";

                $result=DBselect($sql,$elements);

		$table = new CTableInfo();
		$table->SetHeader(array(S_TIMESTAMP,item_description($item["description"],$item["key_"])));
		while($row=DBfetch($result))
		{
			switch($item["value_type"])
			{
				case ITEM_VALUE_TYPE_TEXT:	
					if($DB_TYPE == "ORACLE")
					{
						if(isset($row["value"]))
						{
							$row["value"] = $row["value"]->load();
						}
						else
						{
							$row["value"] = "";
						}
					}
					/* do not use break */
				case ITEM_VALUE_TYPE_STR:	
					$value = nbsp(htmlspecialchars($row["value"]));
					break;
				
				default:
					$value = $row["value"];
					break;
			}

			if($row["valuemapid"] > 0)
				$value = replace_value_by_map($value, $row["valuemapid"]);

			$table->AddRow(array(date(S_DATE_FORMAT_YMDHMS,$row["clock"]),	$value));
		}
		return $table;
	}

	# Add event

	function	get_event_by_eventid($eventid)
	{
		$db_events = DBselect("select * from events where eventid=$eventid");
		return DBfetch($db_events);
	}

	# Reset nextcheck for related items

	function	reset_items_nextcheck($triggerid)
	{
		$sql="select itemid from functions where triggerid=$triggerid";
		$result=DBselect($sql);
		while($row=DBfetch($result))
		{
			$sql="update items set nextcheck=0 where itemid=".$row["itemid"];
			DBexecute($sql);
		}
	}

	# Update configuration

	function	update_config($event_history,$alert_history,$refresh_unsupported,$work_period)
	{
		if(validate_period($work_period) != 0)
		{
			error("Icorrect work period");
			return NULL;
		}

		return	DBexecute("update config set event_history=$event_history,alert_history=$alert_history,".
			" refresh_unsupported=$refresh_unsupported,".
			" work_period=".zbx_dbstr($work_period));
	}

	function	show_table_header($col1, $col2=SPACE)
	{
		$table = new CTable(NULL,"header");
		$table->SetCellSpacing(0);
		$table->SetCellPadding(1);
		$table->AddRow(array(new CCol($col1,"header_l"), new CCol($col2,"header_r")));
		$table->Show();
	}

	# Show History Graph

	function	show_history($itemid,$from,$period)
	{
		$till=date(S_DATE_FORMAT_YMDHMS,time(NULL)-$from*3600);   
		show_table_header("TILL $till (".($period/3600)." HOURs)");

		echo "<center>";
		echo "<TABLE BORDER=0 WIDTH=100% BGCOLOR=\"#CCCCCC\" cellspacing=1 cellpadding=3>";
		echo "<TR BGCOLOR=#DDDDDD>";
		echo "<TD ALIGN=CENTER>";

		echo "<script language=\"JavaScript\" type=\"text/javascript\">";
		echo "if (navigator.appName == \"Microsoft Internet Explorer\")";
		echo "{";
		echo " document.write(\"<IMG SRC='chart.php?itemid=$itemid&period=$period&from=$from&width=\"+(document.body.clientWidth-108)+\"'>\")";
		echo "}";
		echo "else if (navigator.appName == \"Netscape\")";
		echo "{";
		echo " document.write(\"<IMG SRC='chart.php?itemid=$itemid&period=$period&from=$from&width=\"+(document.width-108)+\"'>\")";
		echo "}";
		echo "else";
		echo "{";
		echo " document.write(\"<IMG SRC='chart.php?itemid=$itemid&period=$period&from=$from'>\")";
		echo "}";
		echo "</script>";

		echo "</TD>";
		echo "</TR>";
		echo "</TABLE>";
		echo "</center>";
	}

	function	get_status()
	{
		global $DB_TYPE;
		$status = array();
// server
		if( (exec("ps -ef|grep zabbix_server|grep -v grep|wc -l")>0) || (exec("ps -ax|grep zabbix_server|grep -v grep|wc -l")>0) )
		{
			$status["zabbix_server"] = S_YES;
		}
		else
		{
			$status["zabbix_server"] = S_NO;
		}

// history & trends
		if ($DB_TYPE == "MYSQL")
		{
			$row=DBfetch(DBselect("show table status like 'history'"));
			$status["history_count"]  = $row["Rows"];
			$row=DBfetch(DBselect("show table status like 'history_log'"));
			$status["history_count"] += $row["Rows"];
			$row=DBfetch(DBselect("show table status like 'history_str'"));
			$status["history_count"] += $row["Rows"];
			$row=DBfetch(DBselect("show table status like 'history_uint'"));
			$status["history_count"] += $row["Rows"];

			$row=DBfetch(DBselect("show table status like 'trends'"));
			$status["trends_count"] = $row["Rows"];
		}
		else
		{
			$row=DBfetch(DBselect("select count(itemid) as cnt from history"));
			$status["history_count"]  = $row["cnt"];
			$row=DBfetch(DBselect("select count(itemid) as cnt from history_log"));
			$status["history_count"] += $row["cnt"];
			$row=DBfetch(DBselect("select count(itemid) as cnt from history_str"));
			$status["history_count"] += $row["cnt"];
			$row=DBfetch(DBselect("select count(itemid) as cnt from history_uint"));
			$status["history_count"] += $row["cnt"];

			$result=DBselect("select count(itemid) as cnt from trends");
			$row=DBfetch($result);
			$status["trends_count"]=$row["cnt"];
		}
// events
		$row=DBfetch(DBselect("select count(eventid) as cnt from events"));
		$status["events_count"]=$row["cnt"];
// alerts
		$row=DBfetch(DBselect("select count(alertid) as cnt from alerts"));
		$status["alerts_count"]=$row["cnt"];
// triggers
		$sql = "select count(t.triggerid) as cnt from triggers t, functions f, items i, hosts h".
			" where t.triggerid=f.triggerid and f.itemid=i.itemid and i.status=0 and i.hostid=h.hostid and h.status=".HOST_STATUS_MONITORED;
		$row=DBfetch(DBselect($sql));
		$status["triggers_count"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and t.status=0"));
		$status["triggers_count_enabled"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and t.status=1"));
		$status["triggers_count_disabled"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and t.status=0 and t.value=0"));
		$status["triggers_count_off"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and t.status=0 and t.value=1"));
		$status["triggers_count_on"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and t.status=0 and t.value=2"));
		$status["triggers_count_unknown"]=$row["cnt"];
// items 
		$sql = "select count(i.itemid) as cnt from items i, hosts h where i.hostid=h.hostid and h.status=".HOST_STATUS_MONITORED;
		$row=DBfetch(DBselect($sql));
		$status["items_count"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and i.status=0"));
		$status["items_count_monitored"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and i.status=1"));
		$status["items_count_disabled"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and i.status=3"));
		$status["items_count_not_supported"]=$row["cnt"];

		$row=DBfetch(DBselect($sql." and i.type=2"));
		$status["items_count_trapper"]=$row["cnt"];
// hosts
		$row=DBfetch(DBselect("select count(hostid) as cnt from hosts"));
		$status["hosts_count"]=$row["cnt"];

		$row=DBfetch(DBselect("select count(hostid) as cnt from hosts where status=".HOST_STATUS_MONITORED));
		$status["hosts_count_monitored"]=$row["cnt"];

		$row=DBfetch(DBselect("select count(hostid) as cnt from hosts where status=".HOST_STATUS_NOT_MONITORED));
		$status["hosts_count_not_monitored"]=$row["cnt"];

		$row=DBfetch(DBselect("select count(hostid) as cnt from hosts where status=".HOST_STATUS_TEMPLATE));
		$status["hosts_count_template"]=$row["cnt"];

		$row=DBfetch(DBselect("select count(hostid) as cnt from hosts where status=".HOST_STATUS_DELETED));
		$status["hosts_count_deleted"]=$row["cnt"];
// users
		$row=DBfetch(DBselect("select count(userid) as cnt from users"));
		$status["users_count"]=$row["cnt"];
		
		$status["users_online"]=0;
		$result=DBselect("select distinct s.userid from sessions s, users u where u.userid=s.userid and (s.lastaccess+u.autologout)>".time());
		while(DBfetch($result))		$status["users_online"]++;

		return $status;
	}

	function	get_resource_name($permission,$id)
	{
		$res="-";
		if($permission=="Graph")
		{
			if(isset($id)&&($id!=0))
			{
				if($graph=get_graph_by_graphid($id))
					$res=$graph["name"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All graphs";
			}
		}
		else if($permission=="Host")
		{
			if(isset($id)&&($id!=0))
			{
				if($host=get_host_by_hostid($id))
					$res=$host["host"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All hosts";
			}
		}
		else if($permission=="Screen")
		{
			if(isset($id)&&($id!=0))
			{
				if($screen=get_screen_by_screenid($id))
					$res=$screen["name"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All screens";
			}
		}
		else if($permission=="Item")
		{
			if(isset($id)&&($id!=0))
			{
				if($item=get_item_by_itemid($id))
					if($host=get_host_by_hostid($item["hostid"]))
						$res=$host["host"].":".$item["description"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All items";
			}
		}
		else if($permission=="User")
		{
			if(isset($id)&&($id!=0))
			{
				if($user=get_user_by_userid($id))
					$res=$user["alias"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All users";
			}
		}
		else if($permission=="Network map")
		{
			if(isset($id)&&($id!=0))
			{
				if($user=get_sysmap_by_sysmapid($id))
					$res=$user["name"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All maps";
			}
		}
		else if($permission=="Application")
		{
			if(isset($id)&&($id > 0))
			{
				if($app = get_application_by_applicationid($id))
					$res = $app["name"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All applications";
			}
		}
		else if($permission=="Service")
		{
			if(isset($id)&&($id > 0))
			{
				if($service = get_service_by_serviceid($id))
					$res = $service["name"];
			}
			elseif(!isset($id) || $id == 0)
			{
				$res="All services";
			}
		}

		if($res == '-' && isset($id) && ($id > 0))
			$res = $id;

		return $res;
	}

	function	not_empty($var)
	{
		return ($var == "" ? 0 : 1);
	}

	function	get_cookie($name, $default_value)
	{
		if(isset($_COOKIE[$name]))	return $_COOKIE[$name];
		// else
		return $default_value;
	}
	
	function	get_profile($idx,$default_value,$type=PROFILE_TYPE_UNCNOWN)
	{
		global $USER_DETAILS;

		$result = $default_value;
		if($USER_DETAILS["alias"]!="guest")
		{
			$db_profiles = DBselect("select * from profiles where userid=".$USER_DETAILS["userid"]." and idx=".zbx_dbstr($idx));
			$profile=DBfetch($db_profiles);

			if($profile)
			{
				if($type==PROFILE_TYPE_UNCNOWN)
					$type = $profile["valuetype"];

				$result = $profile["value"];
			}
		}
		switch($type)
		{
			case PROFILE_TYPE_ARRAY:	$result = explode(";", $result); break;
			case PROFILE_TYPE_INT:		$result = intval($result); break;
			case PROFILE_TYPE_STR:		$result = strval($result); break;
		}

		if(is_array($result))
		{
			$result = array_filter($result, "not_empty");
		}
// SDI("Get profile:".$idx." = ".$result);
		return $result;
	}

	function	update_profile($idx,$value,$type=PROFILE_TYPE_UNCNOWN)
	{
// SDI("Save profile:".$idx." = ".$value);

		global $USER_DETAILS;

		if($USER_DETAILS["alias"]=="guest")
		{
			return;
		}

		if($type==PROFILE_TYPE_UNCNOWN && is_array($value))	$type = PROFILE_TYPE_ARRAY;
		if($type==PROFILE_TYPE_ARRAY && !is_array($value))	$value = array($value);

		switch($type)
		{
			case PROFILE_TYPE_ARRAY:	$value = implode(";", $value); break;
			default:			$value = strval($value);
		}

		$row = DBfetch(DBselect("select value from profiles where userid=".$USER_DETAILS["userid"]." and idx=".zbx_dbstr($idx)));

		if(!$row)
		{
			$profileid = get_dbid('profiles', 'profileid');
			$sql="insert into profiles (profileid,userid,idx,value,valuetype)".
				" values (".$profileid.",".$USER_DETAILS["userid"].",".zbx_dbstr($idx).",".zbx_dbstr($value).",".$type.")";
			DBexecute($sql);
		}
		else
		{
			$sql="update profiles set value=".zbx_dbstr($value).",valuetype=".$type.
				" where userid=".$USER_DETAILS["userid"]." and idx=".zbx_dbstr($idx);
			DBexecute($sql);
		}
	}

        function get_drawtype_description($drawtype)
        {
		if($drawtype==0)
			return "Line";
		if($drawtype==1)
			return "Filled region";
		if($drawtype==2)
			return "Bold line";
		if($drawtype==3)
			return "Dot";
		if($drawtype==4)
			return "Dashed line";
		return "Unknown";
        }

$SHOW_HINT_SCRIPT_ISERTTED = false; /* TODO rewrite with JS include */

	function insert_showhint_javascript()
	{
		global $SHOW_HINT_SCRIPT_ISERTTED;

		if($SHOW_HINT_SCRIPT_ISERTTED) return;
		$SHOW_HINT_SCRIPT_ISERTTED = true;
?>
<script language="JavaScript" type="text/javascript">
<!--

function GetPos(obj)
{
	var left = obj.offsetLeft;
	var top  = obj.offsetTop;;
	while (obj = obj.offsetParent)
	{
		left	+= obj.offsetLeft
		top	+= obj.offsetTop
	}
	return [left,top];
}

var hint_box = null;

function hide_hint()
{
	if(!hint_box) return;

	hint_box.style.visibility="hidden"
	//hint_box.style.width	= "0px"
	hint_box.style.left	= "-" + hint_box.style.width;
}

function show_hint(obj, hint_text)
{
	show_hint_ext(obj, hint_text, "", "");
}

function show_hint_ext(obj, hint_text, width, class)
{
	if(!hint_box) return;
	
	if(class != "")
	{
		hint_text = "<span class=" + class + ">" + hint_text + "</span>";
	}

	hint_box.innerHTML = hint_text;
	hint_box.style.width = width;

	var pos = GetPos(obj);

	hint_box.x	= pos[0];
	hint_box.y	= pos[1];

	hint_box.style.left	= hint_box.x + obj.offsetWidth + 10 + "px";
	hint_box.style.top	= hint_box.y + obj.offsetHeight + "px";

	hint_box.style.visibility = "visible";
	obj.onmouseout	= hide_hint;
}

function create_hint_box()
{
	if(hint_box) return;

	hint_box = document.createElement("div");
	hint_box.setAttribute("id", "hint_box");
	document.body.appendChild(hint_box);

	hide_hint();
}

if (window.addEventListener)
{
	window.addEventListener("load", create_hint_box, false);
}
else if (window.attachEvent)
{
	window.attachEvent("onload", create_hint_box);
}
else if (document.getElementById)
{
	window.onload	= create_hint_box;
}
//-->
</script>
<?php
	}

	function insert_confirm_javascript()
	{
?>
<script language="JavaScript" type="text/javascript">
<!--
	function Redirect(url) {
		window.location = url;
		return false;
	}	

	function create_var(form_name, var_name, var_val, submit)
	{
		var frmForm = document.forms[form_name];

		if(!frmForm) return false;

		var objVar = document.createElement('input');

		if(!objVar) return false;

		objVar.setAttribute('type', 	'hidden');
		objVar.setAttribute('name', 	var_name);
		objVar.setAttribute('value', 	var_val);

		frmForm.appendChild(objVar);
		if(submit)
			frmForm.submit();

		return false;
	}

	function Confirm(msg)
	{
		if(confirm(msg,'title'))
			return true;
		else
			return false;
	}
	function PopUp(url,form_name,param)
	{
		window.open(url,form_name,param);
		return false;
	}

	function CheckAll(form_name, chkMain)
	{
		var frmForm = document.forms[form_name];
		var value = frmForm.elements[chkMain].checked;
		for (var i=0; i < frmForm.length; i++)
		{
			if(frmForm.elements[i].type != 'checkbox') continue;
			if(frmForm.elements[i].name == chkMain) continue;
			if(frmForm.elements[i].disabled == true) continue;
			frmForm.elements[i].checked = value;
		}
	}
//-->
</script>
<?php
	}

	function Redirect($url)
	{
?>
<script language="JavaScript" type="text/javascript">
<!--
	window.location = '<?php echo $url; ?>';
//-->
</script>
<?php
	}

	function	SetFocus($frm_name, $fld_name)
	{
?>
<script language="JavaScript" type="text/javascript">
<!--
	document.forms["<?php echo $frm_name; ?>"].elements["<?php echo $fld_name; ?>"].focus();
//-->
</script>
<?php
	}

/* Use ImageSetStyle+ImageLIne instead of bugged ImageDashedLine */
	if(function_exists("imagesetstyle"))
	{
		function DashedLine($image,$x1,$y1,$x2,$y2,$color)
		{
// Style for dashed lines
//			$style = array($color, $color, $color, $color, IMG_COLOR_TRANSPARENT, IMG_COLOR_TRANSPARENT, IMG_COLOR_TRANSPARENT, IMG_COLOR_TRANSPARENT);
			$style = array($color, $color, IMG_COLOR_TRANSPARENT, IMG_COLOR_TRANSPARENT);
			ImageSetStyle($image, $style);
			ImageLine($image,$x1,$y1,$x2,$y2,IMG_COLOR_STYLED);
		}

	}
	else
	{
		function DashedLine($image,$x1,$y1,$x2,$y2,$color)
		{
			ImageDashedLine($image,$x1,$y1,$x2,$y2,$color);
		}
	}

	function DashedRectangle($image,$x1,$y1,$x2,$y2,$color)
	{
		DashedLine($image, $x1,$y1,$x1,$y2,$color);
		DashedLine($image, $x1,$y2,$x2,$y2,$color);
		DashedLine($image, $x2,$y2,$x2,$y1,$color);
		DashedLine($image, $x2,$y1,$x1,$y1,$color);
	}


	function time_navigator($resource="graphid",$id)
	{
	echo "<TABLE BORDER=0 align=center COLS=2 WIDTH=100% BGCOLOR=\"#CCCCCC\" cellspacing=1 cellpadding=1>";
	echo "<TR BGCOLOR=#FFFFFF>";
	echo "<TD ALIGN=LEFT>";

	echo "<div align=left>";
	echo "<b>".S_PERIOD.":</b>".SPACE;

	$hour=3600;
		
		$a=array(S_1H=>3600,S_2H=>2*3600,S_4H=>4*3600,S_8H=>8*3600,S_12H=>12*3600,
			S_24H=>24*3600,S_WEEK_SMALL=>7*24*3600,S_MONTH_SMALL=>31*24*3600,S_YEAR_SMALL=>365*24*3600);
		foreach($a as $label=>$sec)
		{
			echo "[";
			if($_REQUEST["period"]>$sec)
			{
				$tmp=$_REQUEST["period"]-$sec;
				echo("<A HREF=\"charts.php?period=$tmp".url_param($resource).url_param("stime").url_param("from").url_param("keep").url_param("fullscreen")."\">-</A>");
			}
			else
			{
				echo "-";
			}

			echo("<A HREF=\"charts.php?period=$sec".url_param($resource).url_param("stime").url_param("from").url_param("keep").url_param("fullscreen")."\">");
			echo($label."</A>");

			$tmp=$_REQUEST["period"]+$sec;
			echo("<A HREF=\"charts.php?period=$tmp".url_param($resource).url_param("stime").url_param("from").url_param("keep").url_param("fullscreen")."\">+</A>");

			echo "]".SPACE;
		}

		echo("</div>");

	echo "</TD>";
	echo "<TD BGCOLOR=#FFFFFF WIDTH=15% ALIGN=RIGHT>";
	echo "<b>".nbsp(S_KEEP_PERIOD).":</b>".SPACE;
		if($_REQUEST["keep"] == 1)
		{
			echo("[<A HREF=\"charts.php?keep=0".url_param($resource).url_param("from").url_param("period").url_param("fullscreen")."\">".S_ON_C."</a>]");
		}
		else
		{
			echo("[<A HREF=\"charts.php?keep=1".url_param($resource).url_param("from").url_param("period").url_param("fullscreen")."\">".S_OFF_C."</a>]");
		}
	echo "</TD>";
	echo "</TR>";
	echo "<TR BGCOLOR=#FFFFFF>";
	echo "<TD>";
	if(isset($_REQUEST["stime"]))
	{
		echo "<div align=left>" ;
		echo "<b>".S_MOVE.":</b>".SPACE;

		$day=24;
// $a already defined
//		$a=array("1h"=>1,"2h"=>2,"4h"=>4,"8h"=>8,"12h"=>12,
//			"24h"=>24,"week"=>7*24,"month"=>31*24,"year"=>365*24);
		foreach($a as $label=>$hours)
		{
			echo "[";

			$stime=$_REQUEST["stime"];
			$tmp=mktime(substr($stime,8,2),substr($stime,10,2),0,substr($stime,4,2),substr($stime,6,2),substr($stime,0,4));
			$tmp=$tmp-3600*$hours;
			$tmp=date("YmdHi",$tmp);
			echo("<A HREF=\"charts.php?stime=$tmp".url_param($resource).url_param("period").url_param("keep").url_param("fullscreen")."\">-</A>");

			echo($label);

			$stime=$_REQUEST["stime"];
			$tmp=mktime(substr($stime,8,2),substr($stime,10,2),0,substr($stime,4,2),substr($stime,6,2),substr($stime,0,4));
			$tmp=$tmp+3600*$hours;
			$tmp=date("YmdHi",$tmp);
			echo("<A HREF=\"charts.php?stime=$tmp".url_param($resource).url_param("period").url_param("keep").url_param("fullscreen")."\">+</A>");

			echo "]".SPACE;
		}
		echo("</div>");
	}
	else
	{
		echo "<div align=left>";
		echo "<b>".S_MOVE.":</b>".SPACE;

		$day=24;
// $a already defined
//		$a=array("1h"=>1,"2h"=>2,"4h"=>4,"8h"=>8,"12h"=>12,
//			"24h"=>24,"week"=>7*24,"month"=>31*24,"year"=>365*24);
		foreach($a as $label=>$hours)
		{
			echo "[";
			$tmp=$_REQUEST["from"]+$hours;
			echo("<A HREF=\"charts.php?from=$tmp".url_param($resource).url_param("period").url_param("keep").url_param("fullscreen")."\">-</A>");

			echo($label);

			if($_REQUEST["from"]>=$hours)
			{
				$tmp=$_REQUEST["from"]-$hours;
				echo("<A HREF=\"charts.php?from=$tmp".url_param($resource).url_param("period").url_param("keep").url_param("fullscreen")."\">+</A>");
			}
			else
			{
				echo "+";
			}

			echo "]".SPACE;
		}
		echo("</div>");
	}
	echo "</TD>";
	echo "<TD BGCOLOR=#FFFFFF WIDTH=15% ALIGN=RIGHT>";
//		echo("<div align=left>");
		echo "<form method=\"put\" action=\"charts.php\">";
		echo "<input name=\"graphid\" type=\"hidden\" value=\"".$_REQUEST[$resource]."\" size=12>";
		echo "<input name=\"period\" type=\"hidden\" value=\"".(9*3600)."\" size=12>";
		if(isset($_REQUEST["stime"]))
		{
			echo "<input name=\"stime\" class=\"biginput\" value=\"".$_REQUEST["stime"]."\" size=12>";
		}
		else
		{
			echo "<input name=\"stime\" class=\"biginput\" value=\"yyyymmddhhmm\" size=12>";
		}
		echo SPACE;
		echo "<input class=\"button\" type=\"submit\" name=\"action\" value=\"go\">";
		echo "</form>";
//		echo("</div>");
	echo "</TD>";
	echo "</TR>";
	echo "</TABLE>";
	}

	function	add_mapping_to_valuemap($valuemapid, $mappings)
	{
		DBexecute("delete from mappings where valuemapid=$valuemapid");

		foreach($mappings as $map)
		{
			$result = DBexecute("insert into mappings (valuemapid, value, newvalue)".
				" values (".$valuemapid.",".zbx_dbstr($map["value"]).",".
				zbx_dbstr($map["newvalue"]).")");

			if(!$result)
				return $result;
		}
		return TRUE;
	}

	function	add_valuemap($name, $mappings)
	{
		if(!is_array($mappings))	return FALSE;

		$valuemapid = get_dbid("valuemaps","valuemapid");
		
		$result = DBexecute("insert into valuemaps (valuemapid,name) values ($valuemapid,".zbx_dbstr($name).")");
		if(!$result)
			return $result;

		$result = add_mapping_to_valuemap($valuemapid, $mappings);
		if(!$result){
			delete_valuemap($valuemapid);
		}
		else
		{
			$result = $valuemapid;
		}
		return $result;
	}

	function	update_valuemap($valuemapid, $name, $mappings)
	{
		if(!is_array($mappings))	return FALSE;

		$result = DBexecute("update valuemaps set name=".zbx_dbstr($name).
			" where valuemapid=$valuemapid");

		if(!$result)
			return $result;

		$result = add_mapping_to_valuemap($valuemapid, $mappings);
		if(!$result){
			delete_valuemap($valuemapid);
		}
		return $result;
	}

	function	delete_valuemap($valuemapid)
	{
		DBexecute("delete from mappings where valuemapid=$valuemapid");
		DBexecute("delete from valuemaps where valuemapid=$valuemapid");
		return TRUE;
	}

	function	replace_value_by_map($value, $valuemapid)
	{
		if($valuemapid < 1) return $value;

		$result = DBselect("select newvalue from mappings".
			" where valuemapid=".zbx_dbstr($valuemapid)." and value=".zbx_dbstr($value));
		$row = DBfetch($result);
		if($row)
		{
			return $row["newvalue"]." "."($value)";
		}
		return $value;
	}

	function	Alert($msg)
	{
?>
<script language=\"JavaScript\" type=\"text/javascript\">
<!--
	alert('<? echo $msg; ?>');
//-->
</script>
<?php
	}

	function natksort(&$array) {
		$keys = array_keys($array);
		natcasesort($keys);

		$new_array = array();

		foreach ($keys as $k) {
			$new_array[$k] = $array[$k];
		}

		$array = $new_array;
		return true;
	}

	function	set_image_header($format=IMAGE_FORMAT_DEFAULT)
	{
		if(IMAGE_FORMAT_JPEG == $format)	Header( "Content-type:  image/jpeg"); 
		if(IMAGE_FORMAT_TEXT == $format)	Header( "Content-type:  text/html"); 
		else					Header( "Content-type:  image/png"); 
		Header( "Expires:  Mon, 17 Aug 1998 12:51:50 GMT"); 
	}
	
	function ImageOut($image,$format=IMAGE_FORMAT_DEFAULT)
	{
		if(IMAGE_FORMAT_JPEG == $format)
			ImageJPEG($image);
		else
			ImagePNG($image);
	}
?>
