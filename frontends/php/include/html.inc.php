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
	define("BR","<br/>\n");
	define("SPACE","&nbsp;");
	define("RARR","&rArr;");

	function	bold($str)
	{
		if(is_array($str)){
			foreach($str as $key => $val)
				if(is_string($val))
					 $str[$key] = "<b>$val</b>";
		} elseif(is_string($str)) {
			$str = "<b>$str</b>";
		}
		return $str;
	}

	function	bfirst($str) // mark first symbol of string as bold
	{
		$res = bold($str[0]);
		for($i=1,$max=strlen($str); $i<$max; $i++)	$res .= $str[$i];
		$str = $res;
		return $str;	
	}

	function	nbsp($str)
	{
		return str_replace(" ",SPACE,$str);;
	}

	function form_select($var, $value, $label)
	{
		global $_REQUEST;

		$selected = "";
		if(!is_null($var))
		{
			if(isset($_REQUEST[$var])&&$_REQUEST[$var]==$value)
				$selected = "selected";
		}
		return "<option value=\"$value\" $selected>$label";
	}

	function form_input($name, $value, $size)
	{
		return "<input class=\"biginput\" name=\"$name\" size=$size value=\"$value\">";
	}

	function form_textarea($name, $value, $cols, $rows)
	{
		return "<textarea name=\"$name\" cols=\"$cols\" ROWS=\"$rows\" wrap=\"soft\">$value</TEXTAREA>";
	}

	function url1_param($parameter)
	{
		global $_REQUEST;
	
		if(isset($_REQUEST[$parameter]))
		{
			return "$parameter=".$_REQUEST[$parameter];
		}
		else
		{
			return "";
		}
	}

	function url_param($parameter)
	{
		global $_REQUEST;
	
		$result = "";
		if(isset($_REQUEST[$parameter]))
		{
			if(is_array($_REQUEST[$parameter]))
			{
				foreach($_REQUEST[$parameter] as $par)
					$result .= "&".$parameter."[]=".$par;
			}
			else
			{
				$result = "&".$parameter."=".$_REQUEST[$parameter];
			}
		}
		return $result;
	}

	function table_begin($class="tableinfo")
	{
		echo "<table class=\"$class\" border=0 width=\"100%\" bgcolor='#AAAAAA' cellspacing=1 cellpadding=3>";
		echo "\n";
	}

	function table_header($elements)
	{
		echo "<tr bgcolor='#CCCCCC'>";
		while(list($num,$element)=each($elements))
		{
			echo "<td><b>".$element."</b></td>";
		}
		echo "</tr>";
		echo "\n";
	}

	function table_row($elements, $rownum)
	{
		if($rownum%2 == 1)	{ echo "<TR BGCOLOR=\"#DDDDDD\">"; }
		else			{ echo "<TR BGCOLOR=\"#EEEEEE\">"; }

		while(list($num,$element)=each($elements))
		{
			if(is_array($element)&&isset($element["hide"])&&($element["hide"]==1))	continue;
			if(is_array($element))
			{
				if(isset($element["class"]))
					echo "<td class=\"".$element["class"]."\">".$element["value"]."</td>";
				else
					echo "<td>".$element["value"]."</td>";
			}
			else
			{
				echo "<td>".$element."</td>";
			}
		}
		echo "</tr>";
		echo "\n";
	}


	function table_end()
	{
		echo "</table>";
		echo "\n";
	}

	function table_td($text,$attr)
	{
		echo "<td $attr>$text</td>";
	}

	function table_nodata($text="...")
	{
		echo "<TABLE BORDER=0 align=center WIDTH=\"100%\" BGCOLOR=\"#CCCCCC\" cellspacing=1 cellpadding=3>";
		echo "<TR BGCOLOR=\"#DDDDDD\">";
		echo "<TD ALIGN=CENTER>";
		echo $text;
		echo "</TD>";
		echo "</TR>";
		echo "</TABLE>";
	}
?>
