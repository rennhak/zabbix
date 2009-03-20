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
	
	require_once "include/defines.inc.php";
?>
<?php
	global $ZBX_EXPORT_MAP;
	
	$ZBX_EXPORT_MAP = array(
		XML_TAG_HOST => array(
			'attribures'	=> array(
				'host'		=> 'name'),
			'elements'	=> array(
				'proxy'		=> '',
				'useip'		=> '',
				'dns'		=> '',
				'ip'		=> '',
				'port'		=> '',
				'status'	=> '')
			),
		XML_TAG_HOSTPROFILE => array(
			'attribures'	=> array(),
			'elements'	=> array(
				'devicetype'=> '',
				'name'		=> '',
				'os'		=> '',
				'serialno' 	=> '',
				'tag' 		=> '',
				'macaddress'=> '',
				'hardware' 	=> '',
				'software' 	=> '',
				'contact' 	=> '',
				'location' 	=> '',
				'notes' 	=> '')
			),
		XML_TAG_HOSTPROFILE_EXT => array(
			'attribures'	=> array(),
			'elements'	=> array(
				'device_alias' 	=> '',
				'device_type' 	=> '',
				'device_chassis' 	=> '',
				'device_os' 	=> '',
				'device_os_short' 	=> '',
					
				'device_hw_arch' 	=> '',
				'device_serial' 	=> '',
				'device_model' 	=> '',
				'device_tag' 	=> '',
				'device_vendor' 	=> '',
				'device_contract' 	=> '',
					
				'device_who' 	=> '',
				'device_status' 	=> '',
				'device_app_01' 	=> '',
				'device_app_02' 	=> '',
				'device_app_03' 	=> '',
				'device_app_04' 	=> '',
				'device_app_05' 	=> '',
				'device_url_1' 	=> '',
				'device_url_2' 	=> '',
				'device_url_3' 	=> '',
				'device_networks' 	=> '',
				'device_notes' 	=> '',
				'device_hardware' 	=> '',
				'device_software' 	=> '',
				'ip_subnet_mask' 	=> '',
				'ip_router' 	=> '',
				'ip_macaddress' 	=> '',
				'oob_ip' 	=> '',
				'oob_subnet_mask' 	=> '',
				'oob_router' 	=> '',
				'date_hw_buy' 	=> '',
				'date_hw_install' 	=> '',
				'date_hw_expiry' 	=> '',
				'date_hw_decomm' 	=> '',
				'site_street_1' 	=> '',
				'site_street_2' 	=> '',
				'site_street_3' 	=> '',
				'site_city' 	=> '',
				'site_state' 	=> '',
				'site_country' 	=> '',
				'site_zip' 	=> '',
				'site_rack' 	=> '',
				'site_notes' 	=> '',
				'poc_1_name' 	=> '',
				'poc_1_email' 	=> '',
				'poc_1_phone_1' 	=> '',
				'poc_1_phone_2' 	=> '',
				'poc_1_cell' 	=> '',
				'poc_1_screen' 	=> '',
				'poc_1_notes' 	=> '',
				'poc_2_name' 	=> '',
				'poc_2_email' 	=> '',
				'poc_2_phone_1' 	=> '',
				'poc_2_phone_2' 	=> '',
				'poc_2_cell' 	=> '',
				'poc_2_screen' 	=> '',
				'poc_2_notes' 	=> '')
			),
		XML_TAG_DEPENDENCY => array(
			'attribures'	=> array(
				'dependency'	=> 'description'),
			'elements'	=> array(
				'depends'		=> '')
			),
		XML_TAG_ITEM => array(
			'attribures'	=> array(
				'type'			=> '',
				'key_'			=> 'key',
				'value_type'		=> ''),
			'elements'	=> array(
				'description'		=> '',
				'ipmi_sensor'		=> '',
				'delay'			=> '',
				'history'		=> '',
				'trends'		=> '',
				'status'		=> '',
				'units'			=> '',
				'multiplier'		=> '',
				'delta'			=> '',
				'formula'		=> '',
				'lastlogsize'		=> '',
				'logtimefmt'		=> '',
				'delay_flex'		=> '',
				'params'		=> '',
				'trapper_hosts'		=> '',
				'snmp_community'	=> '',
				'snmp_oid'		=> '',
				'snmp_port'		=> '',
				'snmpv3_securityname'	=> '',
				'snmpv3_securitylevel'	=> '',
				'snmpv3_authpassphrase'	=> '',
				'snmpv3_privpassphrase'	=> '')
			),
		XML_TAG_TRIGGER => array(
			'attribures'	=> array(),
			'elements'	=> array(
				'description'		=> '',
				'type'				=> '',
				'expression'		=> '',
				'url'			=> '',
				'status'		=> '',
				'priority'		=> '',
				'comments'		=> '')
			),
		XML_TAG_GRAPH => array(
			'attribures'	=> array(
				'name'			=> '',
				'width'			=> '',
				'height'		=> ''),
			'elements'	=> array(
				'yaxistype'		=> '',
				'show_work_period'	=> '',
				'show_triggers'		=> '',
				'graphtype'		=> '',
				'yaxismin'		=> '',
				'yaxismax'		=> '',
				'show_legend'	=> '',
				'show_3d'		=> '',
				'percent_left'	=> '',
				'percent_right' => '')
			),
		XML_TAG_GRAPH_ELEMENT => array(
			'attribures'	=> array(
				'item'	=> ''),
			'elements'	=> array(
				'drawtype'	=> '',
				'sortorder'	=> '',
				'color'		=> '',
				'yaxisside'	=> '',
				'calc_fnc'	=> '',
				'type'		=> '',
				'periods_cnt'	=> '')
			)
		);

	function zbx_xmlwriter_open_memory(){
		return array('tabs' => 0, 'tag'=>array());
	}
	
	function zbx_xmlwriter_set_indent($mem, $val){
		return true;
	}
	
	function zbx_xmlwriter_start_document(&$mem, $ver){
		echo '<?xml version="'.$ver.'"?';
		return true;
	}
	
	function zbx_xmlwriter_start_element(&$mem, $tag){
		array_push($mem['tag'], $tag);
		echo '>'."\n".str_repeat("\t",$mem['tabs']).'<'.$tag;
		$mem['tabs']++;
		return true;
	}
	
	function zbx_xmlwriter_write_attribute(&$mem, $name, $val){
		echo ' '.$name.'="'.htmlspecialchars($val).'"';
		return true;
	}
	
	function zbx_xmlwriter_end_element(&$mem){
		$teg = array_pop($mem['tag']);
		$mem['tabs']--;
		echo '>'."\n".str_repeat("\t",$mem['tabs']).'</'.$teg;
	}
	
	function zbx_xmlwriter_output_memory(&$mem, $val){ 
/* NOTE: use this function only in the end of xml file creation */
		echo '>'."\n";
	}
	
	function zbx_xmlwriter_write_element(&$mem, $name, $val){
		echo '>'."\n".str_repeat("\t",$mem['tabs']).'<'.$name.'>'.htmlspecialchars($val).'</'.$name;
	}
	
	class CZabbixXMLExport{
		function CZabbixXMLExport(){}
		
		function export_item(&$memory, $itemid){
			global $ZBX_EXPORT_MAP;

			$data = DBfetch(DBselect('select * from items where itemid='.$itemid));
			if(!$data) return false;
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_ITEM);

			$map =& $ZBX_EXPORT_MAP[XML_TAG_ITEM];
			
			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			foreach($map['elements'] as $db_name => $xml_name){
				if(!isset($data[$db_name])) continue;
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
			}

			if( !empty($data['valuemapid']) && $valuemap = DBfetch(DBselect('select name from valuemaps where valuemapid='.$data['valuemapid']))){
				zbx_xmlwriter_write_element($memory, 'valuemap', $valuemap['name']);
			}

			$db_applications=DBselect('SELECT DISTINCT a.name '.
						' FROM applications a,items_applications ia '.
						' WHERE ia.applicationid=a.applicationid and ia.itemid='.$itemid);
			if($application = DBfetch($db_applications)){
				zbx_xmlwriter_start_element ($memory,XML_TAG_APPLICATIONS);
				do{
					zbx_xmlwriter_write_element ($memory, XML_TAG_APPLICATION, $application['name']);
				} while($application = DBfetch($db_applications));
				zbx_xmlwriter_end_element($memory); // XML_TAG_APPLICATIONS
			}

			zbx_xmlwriter_end_element($memory); // XML_TAG_ITEM
		}

		
		function export_trigger(&$memory, $triggerid){
			global $ZBX_EXPORT_MAP;

			$data = DBfetch(DBselect('select * from triggers where triggerid='.$triggerid));
			if(!$data) return false;

			$data['expression'] = explode_exp($data["expression"],0,true);
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_TRIGGER);
			
			$map =& $ZBX_EXPORT_MAP[XML_TAG_TRIGGER];
			
			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			foreach($map['elements'] as $db_name => $xml_name){
				if(!isset($data[$db_name])) continue;
				if(empty($xml_name)) $xml_name = $db_name;
				
				zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
			}
			zbx_xmlwriter_end_element($memory); // XML_TAG_TRIGGER
		}
		
		function export_graph_element(&$memory, $gitemid){
			global $ZBX_EXPORT_MAP;

			$sql = 'select gi.*,i.key_,h.host '.
					' from graphs_items gi, items i, hosts h'.
					' where h.hostid=i.hostid '.
						' and i.itemid=gi.itemid '.
						' and gi.gitemid='.$gitemid;
						
			$data = DBfetch(DBselect($sql));
			if(!$data) return false;

			$data['item'] = $data['host'].':'.$data['key_'];
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_GRAPH_ELEMENT);
			
			$map =& $ZBX_EXPORT_MAP[XML_TAG_GRAPH_ELEMENT];
			
			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			foreach($map['elements'] as $db_name => $xml_name){
				if(!isset($data[$db_name])) continue;
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
			}
			zbx_xmlwriter_end_element($memory); // XML_TAG_GRAPH_ELEMENT
		}

		function export_graph(&$memory, $graphid){
			global $ZBX_EXPORT_MAP;

			$data = DBfetch(DBselect('select * from graphs where graphid='.$graphid));
			if(!$data) return false;
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_GRAPH);
			
			$map =& $ZBX_EXPORT_MAP[XML_TAG_GRAPH];
			
			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			foreach($map['elements'] as $db_name => $xml_name){
				if(!isset($data[$db_name])) continue;
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
			}
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_GRAPH_ELEMENTS);
			
			$db_elements = DBselect('select gitemid from graphs_items where graphid='.$graphid);
			while($element = DBfetch($db_elements)){
				$this->export_graph_element($memory, $element['gitemid']);
			}
				
			zbx_xmlwriter_end_element($memory); // XML_TAG_GRAPH_ELEMENTS
			zbx_xmlwriter_end_element($memory); // XML_TAG_GRAPH
		}
		
		function export_host(&$memory, $hostid, $export_templates, $export_items, $export_triggers, $export_graphs){
			global $ZBX_EXPORT_MAP;

			$data = DBfetch(DBselect('SELECT * FROM hosts WHERE hostid='.$hostid));
			if(!$data){
				return false;
			}
			else if($data['proxy_hostid'] > 0){
				if($proxy = DBfetch(DBselect('SELECT host FROM hosts WHERE hostid='.$data['proxy_hostid'].' AND status='.HOST_STATUS_PROXY))){
					$data['proxy'] = $proxy['host'];
				}
				else{
					$data['proxy'] = '';
				}
			}
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_HOST);
			
			$map =& $ZBX_EXPORT_MAP[XML_TAG_HOST];

			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			foreach($map['elements'] as $db_name => $xml_name){
				if(!isset($data[$db_name])) continue;
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
			}
			
			$sql = 'SELECT g.name '.
					' FROM groups g, hosts_groups hg '.
					' WHERE g.groupid=hg.groupid '.
						' AND hg.hostid='.$hostid;
			if($db_groups = DBselect($sql)){
				zbx_xmlwriter_start_element ($memory,XML_TAG_GROUPS);
				
				while($group = DBfetch($db_groups)){
					zbx_xmlwriter_write_element ($memory, XML_TAG_GROUP, $group['name']);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_GROUP
			}
// based on  mod by scricca
			$data = DBfetch(DBselect('SELECT hp.* FROM hosts_profiles hp WHERE hp.hostid='.$hostid));
			if($data){
				zbx_xmlwriter_start_element ($memory,XML_TAG_HOSTPROFILE);
				
				$map =& $ZBX_EXPORT_MAP[XML_TAG_HOSTPROFILE];
	
				foreach($map['attribures'] as $db_name => $xml_name){
					if(empty($xml_name)) $xml_name = $db_name;
					zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
				}
				
				foreach($map['elements'] as $db_name => $xml_name){
					if(empty($data[$db_name])) continue;
					if(empty($xml_name)) $xml_name = $db_name;
					zbx_xmlwriter_write_element($memory, $xml_name, $data[$db_name]);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_HOSTPROFILE
			}
//--

// Extended profiles
			$data = DBfetch(DBselect('SELECT hpe.* FROM hosts_profiles_ext hpe WHERE hpe.hostid='.$hostid));
			if($data){
				zbx_xmlwriter_start_element($memory,XML_TAG_HOSTPROFILE_EXT);
				
				$map =& $ZBX_EXPORT_MAP[XML_TAG_HOSTPROFILE_EXT];
	
				foreach($map['attribures'] as $db_name => $xml_name){
					if(empty($xml_name)) $xml_name = $db_name;
					zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
				}
				
				foreach($map['elements'] as $db_name => $xml_name){
					if(empty($data[$db_name])) continue;
					if(empty($xml_name)) $xml_name = $db_name;
					zbx_xmlwriter_write_element($memory, $xml_name, $data[$db_name]);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_HOSTPROFILE_EXT
			}
//--
			
			if($export_templates){
				$sql = 'SELECT t.host '.
					' FROM hosts t, hosts_templates ht '.
					' WHERE t.hostid=ht.templateid '.
						' AND ht.hostid='.$hostid;
							
				if($db_templates = DBselect($sql)){
					zbx_xmlwriter_start_element ($memory,XML_TAG_TEMPLATES);

					while($template = DBfetch($db_templates)){
						zbx_xmlwriter_write_element ($memory, XML_TAG_TEMPLATE, $template['host']);
					}
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_TEMPLATES
			}

			if($export_items){
				zbx_xmlwriter_start_element ($memory,XML_TAG_ITEMS);
				
				$db_items=DBselect('select itemid from items where hostid='.$hostid);
				while($item_id = DBfetch($db_items)){
					$this->export_item($memory, $item_id['itemid']);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_ITEMS
			}
			
			if($export_triggers){
				zbx_xmlwriter_start_element ($memory,XML_TAG_TRIGGERS);
				
				$db_triggers = DBselect('select f.triggerid, i.hostid, count(distinct i.hostid) as cnt '.
					' from functions f, items i '.
					' where f.itemid=i.itemid '.
					' group by f.triggerid, i.hostid');
				while($trigger = DBfetch($db_triggers)){
					if((bccomp($trigger['hostid'] , $hostid) != 0) || $trigger['cnt']!=1) continue;
					$this->export_trigger($memory, $trigger['triggerid']);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_TRIGGERS
			}
			
			if($export_graphs){
				zbx_xmlwriter_start_element ($memory, XML_TAG_GRAPHS);
				
				$db_graphs = DBselect('select gi.graphid, i.hostid, count(distinct i.hostid) as cnt '.
					' from graphs_items gi, items i '.
					' where gi.itemid=i.itemid '.
					' group by gi.graphid, i.hostid');
				while($graph = DBfetch($db_graphs)){
					if((bccomp($graph['hostid'] , $hostid) != 0) || $graph['cnt']!=1) continue;
					$this->export_graph($memory, $graph['graphid']);
				}
				
				zbx_xmlwriter_end_element($memory); // XML_TAG_GRAPHS
			}
			/* export screens */
			zbx_xmlwriter_end_element($memory); // XML_TAG_HOST
			return true;
		}
		
		function export_dependency(&$memory, $triggerid, $host, $description){
			global $ZBX_EXPORT_MAP;
			if(!$triggerid) return false;			
			
			$data['dependency'] = $host.':'.$description;
			
			zbx_xmlwriter_start_element ($memory,XML_TAG_DEPENDENCY);
			
			$map =& $ZBX_EXPORT_MAP[XML_TAG_DEPENDENCY];

			foreach($map['attribures'] as $db_name => $xml_name){
				if(empty($xml_name)) $xml_name = $db_name;
				zbx_xmlwriter_write_attribute($memory, $xml_name, $data[$db_name]);
			}
			
			$sql = 'SELECT t.triggerid, t.description, h.host'.
					' FROM trigger_depends td, triggers t, items i, hosts h, functions f '.
					' WHERE td.triggerid_down='.$triggerid.
						' AND t.triggerid=td.triggerid_up '.
						' AND f.triggerid=t.triggerid '.
						' AND i.itemid=f.itemid '.
						' AND h.hostid=i.hostid '.
					' GROUP BY t.triggerid, t.description';			

			$res = DBselect($sql);
			while($data = DBfetch($res)){
				$data['depends'] = $data['host'].':'.$data['description'];
				foreach($map['elements'] as $db_name => $xml_name){
					if(!isset($data[$db_name])) continue;
					if(empty($xml_name)) $xml_name = $db_name;
					zbx_xmlwriter_write_element ($memory, $xml_name, $data[$db_name]);
				}
			}
//--
			/* export screens */
			zbx_xmlwriter_end_element($memory); // XML_TAG_DEPENDENCY
			return true;
		}

		function Export(&$hosts, &$templates, &$items, &$triggers, &$graphs){
		
			$memory = zbx_xmlwriter_open_memory();
			zbx_xmlwriter_set_indent($memory, true);
			zbx_xmlwriter_start_document($memory,'1.0');
			zbx_xmlwriter_start_element ($memory,XML_TAG_ZABBIX_EXPORT);
			zbx_xmlwriter_write_attribute($memory, 'version', '1.0');
			zbx_xmlwriter_write_attribute($memory, 'date', date('d.m.y'));
			zbx_xmlwriter_write_attribute($memory, 'time', date('H.i'));
			
				zbx_xmlwriter_start_element ($memory,XML_TAG_HOSTS);
				foreach($hosts as $hostid => $val){
					$this->export_host(
						$memory,
						$hostid,
						isset($templates[$hostid]),
						isset($items[$hostid]),
						isset($triggers[$hostid]),
						isset($graphs[$hostid])
						);
				}
				zbx_xmlwriter_end_element($memory); // XML_TAG_HOSTS
				
				if(!empty($triggers)){

					zbx_xmlwriter_start_element ($memory,XML_TAG_DEPENDENCIES);
					foreach($triggers as $hostid => $val){
						
						$sql = 'SELECT h.host, t.description, t.triggerid '.
								' FROM trigger_depends td, triggers t, functions f, items i, hosts h '.
								' WHERE h.hostid='.$hostid.
									' AND i.hostid=h.hostid '.
									' AND f.itemid=i.itemid '.
									' AND t.triggerid=f.triggerid '.
									' AND td.triggerid_down=t.triggerid '.
								' GROUP BY t.triggerid, t.description';

						$dependent_triggers = DBselect($sql);
						while($deps = DBfetch($dependent_triggers)){
							$this->export_dependency(
								$memory,
								$deps['triggerid'],
								$deps['host'],
								$deps['description']
								);						
						}
					}
					zbx_xmlwriter_end_element($memory); // XML_TAG_DEPENDENCIES
				}
				
			zbx_xmlwriter_end_element($memory); // XML_TAG_ZABBIX_EXPORT
			die(zbx_xmlwriter_output_memory($memory,true));
		}
	}
?>
