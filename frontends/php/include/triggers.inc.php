<?php
/*
** ZABBIX
** Copyright (C) 2000-2007 SIA Zabbix
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
	require_once('maps.inc.php');
	require_once('acknow.inc.php');
	require_once('services.inc.php');

	/*
	 * Function: INIT_TRIGGER_EXPRESSION_STRUCTURES
	 *
	 * Description: 
	 *     initialize structures for trigger expression
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments:
	 *
	 */
	function INIT_TRIGGER_EXPRESSION_STRUCTURES(){
		if ( defined('TRIGGER_EXPRESSION_STRUCTURES_OK') ) return;
		define('TRIGGER_EXPRESSION_STRUCTURES_OK', 1);

		global $ZBX_TR_EXPR_ALLOWED_MACROS, $ZBX_TR_EXPR_REPLACE_TO, $ZBX_TR_EXPR_ALLOWED_FUNCTIONS;

		$ZBX_TR_EXPR_ALLOWED_MACROS['{TRIGGER.VALUE}'] = '{TRIGGER.VALUE}';

		$ZBX_TR_EXPR_REPLACE_TO = 'zbx_expr_ok';

		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['abschange']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['avg']	= array('args' => array(	0 => array('type' => 'sec_num','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				),
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['delta']	= array('args' => array( 0 => array('type' => 'sec_num','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				),
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['change']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				),
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['count']	= array('args' => array( 0 => array('type' => 'sec_num','mandat' => true), 1 => array('type' => 'str'), 1=>array('type' => 'str') ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_LOG,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['date']	= array('args' => null, 'item_types' => null );

		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['dayofweek']= array('args' => null,	'item_types' => null );

		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['diff']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['fuzzytime']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['iregexp']= array('args' => array( 0 => array('type' => 'str','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_LOG
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['last']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['max']	= array('args' => array( 0 => array('type' => 'sec_num','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['min']	= array('args' => array( 0 => array('type' => 'sec_num','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['nodata']= array('args' => array( 0 => array('type' => 'sec','mandat' => true) ), 'item_types' => null );
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['now']	= array('args' => null, 'item_types' => null );
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['prev']	= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64,
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_TEXT
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['str']	= array('args' => array( 0 => array('type' => 'str','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_LOG
				)
			);


		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['sum']	= array('args' => array( 0 => array('type' => 'sec_num','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_FLOAT,
				ITEM_VALUE_TYPE_UINT64
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['logseverity']= array('args' => null,
			'item_types' => array(
				ITEM_VALUE_TYPE_LOG
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['logsource']= array('args' => array( 0=> array('type' => 'str','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_LOG
				)
			);

		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['regexp']= array('args' => array( 0 => array('type' => 'str','mandat' => true) ),
			'item_types' => array(
				ITEM_VALUE_TYPE_STR,
				ITEM_VALUE_TYPE_LOG
				)
			);
		$ZBX_TR_EXPR_ALLOWED_FUNCTIONS['time']	= array('args' => null, 'item_types' => null );
	}

	INIT_TRIGGER_EXPRESSION_STRUCTURES();
	
/*
 * Function: get_accessible_triggers
 *
 * Description:
 *     returns string of accessible triggers
 *
 * Author:
 *     Aly
 *
 */		
	function get_accessible_triggers($perm,$hostids,$perm_res=null,$nodeid=null,$cache=1){
		global $USER_DETAILS;
		static $available_triggers;
		zbx_value2array($hostids);
		
		$result = array();
		if(is_null($perm_res)) $perm_res = PERM_RES_IDS_ARRAY;
		$nodeid_str =(is_array($nodeid))?implode('',$nodeid):strval($nodeid);
		$hostid_str = implode('',$hostids);

		$cache_hash = md5($perm.$perm_res.$nodeid_str.$hostid_str);
		if($cache && isset($available_triggers[$cache_hash])){
			return $available_triggers[$cache_hash];
		}

		$available_hosts = get_accessible_hosts_by_user($USER_DETAILS, $perm, PERM_RES_IDS_ARRAY, $nodeid);

		$denied_triggers = array();
		$available_triggers = array();
		
		$sql_where = '';
		if(!empty($hostids)){
			$sql_where.= ' AND '.DBcondition('i.hostid',$hostids);
		}		
		$sql = 	'SELECT DISTINCT t.triggerid '.
				' FROM triggers t, functions f, items i '.
				' WHERE t.triggerid=f.triggerid '.
					' AND f.itemid=i.itemid'.
					' AND '.DBcondition('i.hostid',$available_hosts,true).
					$sql_where;
		$db_triggers = DBselect($sql);
		while($trigger = DBfetch($db_triggers)){
			$denied_triggers[] = $trigger['triggerid'];
		}

		if(!empty($denied_triggers)){
			$sql_where.= ' AND '.DBcondition('t.triggerid',$denied_triggers,true);
		}

		$sql = 	'SELECT DISTINCT t.triggerid '.
				' FROM triggers t, functions f, items i '.
				' WHERE t.triggerid=f.triggerid '.
					' AND f.itemid=i.itemid'.
					$sql_where;
		$db_triggers = DBselect($sql);
		while($trigger = DBfetch($db_triggers)){
			$result[$trigger['triggerid']] = $trigger['triggerid'];
		}
		
		if(PERM_RES_STRING_LINE == $perm_res){
			if(count($result) == 0) 
				$result = '-1';
			else
				$result = implode(',',$result);
		}
		
		$available_triggers[$cache_hash] = $result;
		
	return $result;
	}


/*
 * Function: get_severity_style 
 *
 * Description: 
 *     convert severity constant in to the CSS style name
 *     
 * Author: 
 *     Aly
 *
 * Comments:
 *
 */
	function get_severity_style($severity,$type=true){
		switch($severity){
			case TRIGGER_SEVERITY_DISASTER:
				$style='disaster';
				break;
			case TRIGGER_SEVERITY_HIGH:
				$style='high';
				break;
			case TRIGGER_SEVERITY_AVERAGE:
				$style='average';
				break;
			case TRIGGER_SEVERITY_WARNING:
				$style='warning';
				break;
			case TRIGGER_SEVERITY_INFORMATION:
			default:
				$style='information';
		}
		if(!$type) $style='normal';//$style.='_empty';
	return $style;
	}
	
/*
 * Function: get_service_status_of_trigger
 *
 * Description: 
 *     retrive trigger's priority for services
 *     
 * Author: 
 *     Artem Suharev
 *
 * Comments:
 *
 */
	
	function get_service_status_of_trigger($triggerid){
		$sql = 'SELECT triggerid, priority '.
				' FROM triggers '.
				' WHERE triggerid='.$triggerid.
					' AND status='.TRIGGER_STATUS_ENABLED.
					' AND value='.TRIGGER_VALUE_TRUE;

		$status = ($rows=DBfetch(DBselect($sql,1)))?$rows['priority']:0;

	return $status;
	}
	
/*
 * Function: get_severity_description 
 *
 * Description: 
 *     convert severity constant in to the string representation
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function get_severity_description($severity){
		if($severity == TRIGGER_SEVERITY_NOT_CLASSIFIED)	return S_NOT_CLASSIFIED;
		else if($severity == TRIGGER_SEVERITY_INFORMATION)	return S_INFORMATION;
		else if($severity == TRIGGER_SEVERITY_WARNING)		return S_WARNING;
		else if($severity == TRIGGER_SEVERITY_AVERAGE)		return S_AVERAGE;
		else if($severity == TRIGGER_SEVERITY_HIGH)		return S_HIGH;
		else if($severity == TRIGGER_SEVERITY_DISASTER)		return S_DISASTER;

		return S_UNKNOWN;
	}

/*
 * Function: get_trigger_value_style 
 *
 * Description: 
 *     convert trigger value in to the CSS style name
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function get_trigger_value_style($value){
		$str_val[TRIGGER_VALUE_FALSE]	= 'off';
		$str_val[TRIGGER_VALUE_TRUE]	= 'on';
		$str_val[TRIGGER_VALUE_UNKNOWN]	= 'unknown';

		if(isset($str_val[$value]))
			return $str_val[$value];

		return '';
	}

/*
 * Function: trigger_value2str 
 *
 * Description: 
 *     convert trigger value in to the string representation
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function trigger_value2str($value){
		$str_val[TRIGGER_VALUE_FALSE]	= S_OK_BIG;
		$str_val[TRIGGER_VALUE_TRUE]	= S_PROBLEM_BIG;
		$str_val[TRIGGER_VALUE_UNKNOWN]	= S_UNKNOWN_BIG;

		if(isset($str_val[$value]))
			return $str_val[$value];

		return S_UNKNOWN;
	}

/*
 * Function: get_realhosts_by_triggerid 
 *
 * Description: 
 *     retrive real host for trigger
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function get_realhosts_by_triggerid($triggerid){
		$trigger = get_trigger_by_triggerid($triggerid);
		if($trigger['templateid'] > 0)
			return get_realhosts_by_triggerid($trigger['templateid']);

		return get_hosts_by_triggerid($triggerid);
	}

	function get_trigger_by_triggerid($triggerid){
		$sql="select * from triggers where triggerid=$triggerid";
		$result=DBselect($sql);
		$row=DBfetch($result);
		if($row){
			return	$row;
		}
		error("No trigger with triggerid=[$triggerid]");
		return FALSE;
	}

	function get_hosts_by_triggerid($triggerids){
		zbx_value2array($triggerids);
		
		return DBselect('SELECT DISTINCT h.* '.
						' FROM hosts h, functions f, items i '.
						' WHERE i.itemid=f.itemid '.
							' AND h.hostid=i.hostid '.
							' AND '.DBcondition('f.triggerid',$triggerids));
	}

	function get_functions_by_triggerid($triggerid){
		return DBselect('select * from functions where triggerid='.$triggerid);
	}

/*
 * Function: get_triggers_by_hostid
 *
 * Description: 
 *     retrive selection of triggers by hostid
 *     
 * Author: 
 *    Aly
 *
 * Comments:
 *
 */
	function get_triggers_by_hostid($hostid){
		$db_triggers = DBselect('SELECT DISTINCT t.* '.
								' FROM triggers t, functions f, items i '.
								' WHERE i.hostid='.$hostid.
									' AND f.itemid=i.itemid '.
									' AND f.triggerid=t.triggerid');
	return $db_triggers;
	}
	

/*
 * Function: get_trigger_by_description
 *
 * Description: 
 *     retrive triggerid by description
 *     
 * Author: 
 *    Aly
 *
 * Comments:
 *	  description - host-name:trigger-description. Example( "unix server:low free disk space")
 */
	
	function get_trigger_by_description($desc){
		list($host_name, $trigger_description) = explode(':',$desc);
		
		$sql = 'SELECT t.* '.
				' FROM triggers t, items i, functions f, hosts h '.
				' WHERE h.host='.zbx_dbstr($host_name).
					' AND i.hostid=h.hostid '.
					' AND f.itemid=i.itemid '.
					' AND t.triggerid=f.triggerid '.
					' AND t.description='.zbx_dbstr($trigger_description).
				' ORDER BY t.triggerid DESC';
		$trigger = DBfetch(DBselect($sql,1));
	return $trigger;
	}

	function get_triggers_by_templateid($triggerids){
		zbx_value2array($triggerids);
	return DBselect('SELECT * FROM triggers WHERE '.DBcondition('templateid',$triggerids));
	}

/*
 * Function: get_hosts_by_expression
 *
 * Description: 
 *     retrive selection of hosts by trigger expression
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function get_hosts_by_expression($expression){
		global $ZBX_TR_EXPR_ALLOWED_MACROS, $ZBX_TR_EXPR_REPLACE_TO;

		$expr = $expression;

		$hosts = array();

		/* Replace all {server:key.function(param)} and {MACRO} with '$ZBX_TR_EXPR_REPLACE_TO' */
		while(ereg(ZBX_EREG_EXPRESSION_TOKEN_FORMAT, $expr, $arr)){
			if($arr[ZBX_EXPRESSION_MACRO_ID] && !isset($ZBX_TR_EXPR_ALLOWED_MACROS[$arr[ZBX_EXPRESSION_MACRO_ID]]) ){
				$hosts = array('0');
				break;
			}
			else if( !$arr[ZBX_EXPRESSION_MACRO_ID] ) {
				$hosts[] = zbx_dbstr($arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_HOST_ID]);
			}
			$expr = $arr[ZBX_EXPRESSION_LEFT_ID].$ZBX_TR_EXPR_REPLACE_TO.$arr[ZBX_EXPRESSION_RIGHT_ID];
		}

		if(count($hosts) == 0) $hosts = array('0');

		$sql = 'SELECT DISTINCT * '.
				' FROM hosts '.
				' WHERE '.DBin_node('hostid', get_current_nodeid(false)).
					' AND status IN ('.HOST_STATUS_MONITORED.','.HOST_STATUS_NOT_MONITORED.','.HOST_STATUS_TEMPLATE.') '.
					' AND host IN ('.implode(',',$hosts).')';
					
	return DBselect($sql);
	}

/*
 * Function: zbx_unquote_param
 *
 * Description: 
 *     unquote string and unescape cahrs
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *     Double quotes used only.
 *     Unquote string only if value directly in quotes.
 *     Unescape only '\\' and '\"' combination
 *
 */
	function zbx_unquote_param($value){
		$value = trim($value);
		if ( !empty($value) && '"' == $value[0] ){ 
/* open quotes and unescape chars */
			$value = substr($value, 1, strlen($value)-2);

			$new_val = '';
			for ( $i=0, $max=strlen($value); $i < $max; $i++){
				if ( $i+1 < $max && $value[$i] == '\\' && ($value[$i+1] == '\\' || $value[$i+1] == '"') )
					$new_val .= $value[++$i];
				else
					$new_val .= $value[$i];
			}
			$value = $new_val;
		}
	return $value;
	}

/*
 * Function: zbx_get_params 
 *
 * Description: 
 *     parse list of quoted parameters
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *     Double quotes used only.
 *
 */
	function zbx_get_params($string){
		$params = array();
		$quoted = false;

		for( $param_s = $i = 0, $len = strlen($string); $i < $len; $i++){
			switch ( $string[$i] ){
				case '"':
					$quoted = !$quoted;
					break;
				case ',':
					if ( !$quoted ){
						$params[] = zbx_unquote_param(substr($string, $param_s, $i - $param_s));
						$param_s = $i+1;
					}
					break;
				case '\\':
					if ( $quoted && $i+1 < $len && ($string[$i+1] == '\\' || $string[$i+1] == '"'))
						$i++;
					break;
			}
		}

		if( $quoted ){
			error('Incorrect usage of quotes. ['.$string.']');
			return null;
		}

		if($i > $param_s){
			$params[] = zbx_unquote_param(substr($string, $param_s, $i - $param_s));
		}

	return $params;
	}

/*
 * Function: validate_expression 
 *
 * Description: 
 *     check trigger expression syntax and validate values
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments:
 *
 */
	function validate_expression($expression){
		global $ZBX_TR_EXPR_ALLOWED_MACROS, $ZBX_TR_EXPR_REPLACE_TO, $ZBX_TR_EXPR_ALLOWED_FUNCTIONS;

		if( empty($expression) ){
			error('Expression can\'t be empty');
		}
		
		$expr = $expression;
		$h_status = array();

		/* Replace all {server:key.function(param)} and {MACRO} with '$ZBX_TR_EXPR_REPLACE_TO' */
		while(ereg(ZBX_EREG_EXPRESSION_TOKEN_FORMAT, $expr, $arr)){
			if ( $arr[ZBX_EXPRESSION_MACRO_ID] && !isset($ZBX_TR_EXPR_ALLOWED_MACROS[$arr[ZBX_EXPRESSION_MACRO_ID]]) ){
				error('Unknown macro ['.$arr[ZBX_EXPRESSION_MACRO_ID].']');
				return false;
			}
			else if( !$arr[ZBX_EXPRESSION_MACRO_ID] ) {
				$host		= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_HOST_ID];
				$key		= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_KEY_ID];
				$function	= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_FUNCTION_NAME_ID];
				$parameter	= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_FUNCTION_PARAM_ID];
				
				/* Check host */
				$sql = 'SELECT COUNT(*) as cnt,min(status) as status,min(hostid) as hostid '.
						' FROM hosts h '.
						' WHERE h.host='.zbx_dbstr($host).
							' AND '.DBin_node('h.hostid', get_current_nodeid(false)).
							' AND status IN ('.HOST_STATUS_MONITORED.','.HOST_STATUS_NOT_MONITORED.','.HOST_STATUS_TEMPLATE.') ';
							
				$row=DBfetch(DBselect($sql));
				if($row['cnt']==0){
					error('No such host ('.$host.')');
					return false;
				}
				else if($row['cnt']!=1){
					error('Too many hosts ('.$host.')');
					return false;
				}

				$h_status[$row['status']][$row['hostid']] = $row['cnt'];

				/* Check key */
				$sql = 'SELECT i.itemid,i.value_type '.
						' FROM hosts h,items i '.
						' WHERE h.host='.zbx_dbstr($host).
							' AND i.key_='.zbx_dbstr($key).
							' AND h.hostid=i.hostid '.
							' AND '.DBin_node('h.hostid', get_current_nodeid(false));
							
				if (!$item = DBfetch(DBselect($sql)) ){
					error('No such monitored parameter ('.$key.') for host ('.$host.')');
					return false;
				}

				/* Check function */
				if( !isset($ZBX_TR_EXPR_ALLOWED_FUNCTIONS[$function]) ){
					error('Unknown function ['.$function.']');
					return false;
				}

				$fnc_valid = &$ZBX_TR_EXPR_ALLOWED_FUNCTIONS[$function];

				if ( is_array($fnc_valid['item_types']) &&
					!uint_in_array($item['value_type'], $fnc_valid['item_types'])){
					$allowed_types = array();
					foreach($fnc_valid['item_types'] as $type)
						$allowed_types[] = item_value_type2str($type);
					info('Function ('.$function.') available only for items with value types ['.implode(',',$allowed_types).']');
					error('Incorrect value type ['.item_value_type2str($item['value_type']).'] for function ('.$function.') of key ('.$host.':'.$key.')');
					return false;
				}

				if( !is_null($fnc_valid['args']) ){
					$parameter = zbx_get_params($parameter);

					if( !is_array($fnc_valid['args']) )
						$fnc_valid['args'] = array($fnc_valid['args']);

					foreach($fnc_valid['args'] as $pid => $params){
						if(!isset($parameter[$pid])){
							if( !isset($params['mandat']) ){
								continue;
							}
						 	else{
								error('Missed mandatory parameter for function ('.$function.')');
								return false;
							}
						}

						if( 'sec' == $params['type'] && (validate_float($parameter[$pid])!=0) ){
							error('['.$parameter[$pid].'] is not a float for function ('.$function.')');
							return false;
						}

						if( 'sec_num' == $params['type'] 
							&& (validate_ticks($parameter[$pid])!=0) ){
							error('['.$parameter[$pid].'] is not a float or counter for function ('.$function.')');
							return false;
						}
					}
				}
			}
			$expr = $arr[ZBX_EXPRESSION_LEFT_ID].$ZBX_TR_EXPR_REPLACE_TO.$arr[ZBX_EXPRESSION_RIGHT_ID];
		}

		if ( isset($h_status[HOST_STATUS_TEMPLATE]) && ( count($h_status) > 1 || count($h_status[HOST_STATUS_TEMPLATE]) > 1 )){
			error("Incorrect trigger expression. You can't use template hosts".
				" in mixed expressions.");
			return false;
		}

		/* Replace all calculations and numbers with '$ZBX_TR_EXPR_REPLACE_TO' */
		$expt_number = '('.$ZBX_TR_EXPR_REPLACE_TO.'|'.ZBX_EREG_NUMBER.')';
		$expt_term = '((\('.$expt_number.'\))|('.$expt_number.'))';
		$expr_format = '(('.$expt_term.ZBX_EREG_SPACES.ZBX_EREG_SIGN.ZBX_EREG_SPACES.$expt_term.')|(\('.$expt_term.'\)))';
		$expr_full_format = '((\('.$expr_format.'\))|('.$expr_format.'))';

		while($res = ereg($expr_full_format.'([[:print:]]*)$', $expr, $arr)){
			$expr = substr($expr, 0, strpos($expr, $arr[1])).$ZBX_TR_EXPR_REPLACE_TO.$arr[58];
		}

		if ( $ZBX_TR_EXPR_REPLACE_TO != $expr ){
			error('Incorrect trigger expression. ['.str_replace($ZBX_TR_EXPR_REPLACE_TO, ' ... ', $expr).']');
			return false;
		}

		return true;
	}


	function add_trigger($expression, $description, $type, $priority, $status, $comments, $url, $deps=array(), $templateid=0){
		if( !validate_expression($expression) )
			return false;

		$triggerid=get_dbid("triggers","triggerid");
		
		$result=DBexecute('INSERT INTO triggers '.
			'  (triggerid,description,type,priority,status,comments,url,value,error,templateid) '.
			" values ($triggerid,".zbx_dbstr($description).",$type,$priority,$status,".zbx_dbstr($comments).','.
			zbx_dbstr($url).",2,'Trigger just added. No status update so far.',$templateid)");
			
		if(!$result){
			return	$result;
		}
 
		add_event($triggerid,TRIGGER_VALUE_UNKNOWN);
 
		if( null == ($expression = implode_exp($expression,$triggerid)) ){
			$result = false;
		}

		if($result){
			DBexecute("update triggers set expression=".zbx_dbstr($expression)." where triggerid=$triggerid");

			reset_items_nextcheck($triggerid);

			foreach($deps as $id => $triggerid_up){
				if(!$result2=add_trigger_dependency($triggerid, $triggerid_up)){
					error(S_INCORRECT_DEPENDENCY.' ['.expand_trigger_description($triggerid_up).']');
				}
				$result &= $result2;
			}
		}

		$trig_hosts = get_hosts_by_triggerid($triggerid);
		$trig_host = DBfetch($trig_hosts);
		
		if($result){
			$msg = "Added trigger '".$description."'";
			if($trig_host){
				$msg .= " to host '".$trig_host["host"]."'";
			}
			info($msg);
		}

		if($trig_host){
// create trigger for childs
			$child_hosts = get_hosts_by_templateid($trig_host["hostid"]);
			while($child_host = DBfetch($child_hosts)){
				if( !($result = copy_trigger_to_host($triggerid, $child_host["hostid"])))
					break;
			}
		}

		if(!$result){
			if($templateid == 0){ 
// delete main trigger (and recursively childs)
				delete_trigger($triggerid);
			}
			return $result;
		}

		return $triggerid;
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function get_trigger_dependencies_by_triggerid($triggerid){
		$result = array();

		$db_deps = DBselect('SELECT * FROM trigger_depends WHERE triggerid_down='.$triggerid);
		while($db_dep = DBfetch($db_deps))
				$result[] = $db_dep['triggerid_up'];
			
	return $result;
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function replace_template_dependencies($deps, $hostid){
		foreach($deps as $id => $val){
			if($db_new_dep = DBfetch(DBselect('SELECT t.triggerid '.
				' FROM triggers t,functions f,items i '.
				' WHERE t.templateid='.$val.
					' AND f.triggerid=t.triggerid '.
					' AND f.itemid=i.itemid '.
					' AND i.hostid='.$hostid)))
			{
					$deps[$id] = $db_new_dep['triggerid'];
			}
		}
	return $deps;
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function copy_trigger_to_host($triggerid, $hostid, $copy_mode = false){
		$trigger = get_trigger_by_triggerid($triggerid);

		$deps = replace_template_dependencies(
				get_trigger_dependencies_by_triggerid($triggerid),
				$hostid);

		$sql='SELECT t2.triggerid, t2.expression '.
				' FROM triggers t2, functions f1, functions f2, items i1, items i2 '.
				' WHERE f1.triggerid='.$triggerid.
					' AND i1.itemid=f1.itemid '.
					' AND f2.function=f1.function '.
					' AND f2.parameter=f1.parameter '.
					' AND i2.itemid=f2.itemid '.
					' AND i2.key_=i1.key_ '.
					' AND i2.hostid='.$hostid.
					' AND t2.triggerid=f2.triggerid '.
					' AND t2.templateid=0 ';
		
		$host_triggers = DBSelect($sql);
		while($host_trigger = DBfetch($host_triggers)){
			if(cmp_triggers_exressions($triggerid, $host_trigger["triggerid"]))	continue;
			// link not linked trigger with same expression
			
			return update_trigger(
				$host_trigger["triggerid"],
				NULL,	// expression
				$trigger["description"],
				$trigger["type"],
				$trigger["priority"],
				NULL,	// status
				$trigger["comments"],
				$trigger["url"],
				$deps,
				$copy_mode ? 0 : $triggerid);
		}

		$newtriggerid=get_dbid('triggers','triggerid');

		$result = DBexecute('INSERT INTO triggers '.
					' (triggerid,description,type,priority,status,comments,url,value,expression,templateid)'.
					' VALUES ('.$newtriggerid.','.zbx_dbstr($trigger['description']).','.$trigger['type'].','.$trigger['priority'].','.
					$trigger["status"].','.zbx_dbstr($trigger["comments"]).','.
					zbx_dbstr($trigger["url"]).",2,'0',".($copy_mode ? 0 : $triggerid).')');

		if(!$result)
			return $result;

		$host = get_host_by_hostid($hostid);
		$newexpression = $trigger["expression"];

		// Loop: functions
		$functions = get_functions_by_triggerid($triggerid);
		while($function = DBfetch($functions)){
			$item = get_item_by_itemid($function["itemid"]);

			$host_items = DBselect('SELECT * FROM items WHERE key_='.zbx_dbstr($item['key_']).' AND hostid='.$host['hostid']);
			$host_item = DBfetch($host_items);
			if(!$host_item){
				error("Missing key '".$item["key_"]."' for host '".$host["host"]."'");
				return FALSE;
			}

			$newfunctionid=get_dbid("functions","functionid");

			$result = DBexecute('INSERT INTO functions (functionid,itemid,triggerid,function,parameter) '.
				" values ($newfunctionid,".$host_item["itemid"].",$newtriggerid,".
				zbx_dbstr($function["function"]).",".zbx_dbstr($function["parameter"]).")");

			$newexpression = str_replace(
				"{".$function["functionid"]."}",
				"{".$newfunctionid."}",
				$newexpression);
		}

		DBexecute('UPDATE triggers SET expression='.zbx_dbstr($newexpression).' WHERE triggerid='.$newtriggerid);
// copy dependencies
		delete_dependencies_by_triggerid($newtriggerid);
		foreach($deps as $dep_id){
			add_trigger_dependency($newtriggerid, $dep_id);
		}

		info("Added trigger '".$trigger["description"]."' to host '".$host["host"]."'");

// Copy triggers to the child hosts
		$child_hosts = get_hosts_by_templateid($hostid);
		while($child_host = DBfetch($child_hosts)){
// recursion
			$result = copy_trigger_to_host($newtriggerid, $child_host["hostid"]);
			if(!$result){
				return result;
			}
		}

	return $newtriggerid;
	}
	

	/******************************************************************************
	 *                                                                            *
	 * Purpose: Translate {10}>10 to something like localhost:procload.last(0)>10 *
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function explode_exp ($expression, $html,$template=false){
//		echo "EXPRESSION:",$expression,"<Br>";

		$functionid='';
		if(0 == $html){
			$exp='';
		}
		else{
			$exp=array();
		}
		
		$state='';
		for($i=0,$max=strlen($expression); $i<$max; $i++){
			if($expression[$i] == '{'){
				$functionid='';
				$state='FUNCTIONID';
				continue;
			}
			
			if($expression[$i] == '}'){
				$state='';
				if($functionid=="TRIGGER.VALUE"){
					if(0 == $html) $exp.='{'.$functionid.'}';
					else array_push($exp,'{'.$functionid.'}');
				}
				else if(is_numeric($functionid) && 
					$function_data = DBfetch(DBselect('SELECT h.host,i.key_,f.function,f.parameter,i.itemid,i.value_type'.
													' FROM items i,functions f,hosts h'.
													' WHERE f.functionid='.$functionid.
														' AND i.itemid=f.itemid '.
														' AND h.hostid=i.hostid'
					)))
				{
					if($template) $function_data["host"] = '{HOSTNAME}';
						
					if($html == 0){
						$exp.='{'.$function_data['host'].':'.$function_data['key_'].'.'.$function_data['function'].'('.$function_data['parameter'].')}';
					}
					else{
						$link = new CLink($function_data['host'].':'.$function_data['key_'],
							'history.php?action='.( (($function_data['value_type'] == ITEM_VALUE_TYPE_FLOAT) || ($function_data['value_type'] == ITEM_VALUE_TYPE_UINT64))?'showgraph':'showvalues').'&itemid='.$function_data['itemid']);
					
						array_push($exp,array('{',$link,'.',bold($function_data['function'].'('),$function_data['parameter'],bold(')'),'}'));
					}
				}
				else{
					if(1 == $html){
						$font = new CTag('font','yes');
						$font->addOption('color','#AA0000');
						$font->addItem('*ERROR*');
						array_push($exp,$font);
					}
					else{
						$exp.= '*ERROR*';
					}

				}
				continue;
			}
			if($state == "FUNCTIONID"){
				$functionid=$functionid.$expression[$i];
				continue;
			}
			
			if(1 == $html) array_push($exp,$expression[$i]);
			else $exp.=$expression[$i];
		}
#		echo "EXP:",$exp,"<Br>";
		return $exp;
	}

	/*
	 * Function: implode_exp
	 *
	 * Description: 
	 *     Translate localhost:procload.last(0)>10 to {12}>10
	 *     And create database representation.
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function implode_exp($expression, $triggerid){
		global $ZBX_TR_EXPR_ALLOWED_MACROS, $ZBX_TR_EXPR_REPLACE_TO;
		$expr = $expression;
		$short_exp = $expression;

		/* Replace all {server:key.function(param)} and {MACRO} with '$ZBX_TR_EXPR_REPLACE_TO' */
		/* build short expression {12}>10 */
		while(ereg(ZBX_EREG_EXPRESSION_TOKEN_FORMAT, $expr, $arr)){
			if($arr[ZBX_EXPRESSION_MACRO_ID] && !isset($ZBX_TR_EXPR_ALLOWED_MACROS[$arr[ZBX_EXPRESSION_MACRO_ID]])){
				error('[ie] Unknown macro ['.$arr[ZBX_EXPRESSION_MACRO_ID].']');
				return false;
			}
			else if(!$arr[ZBX_EXPRESSION_MACRO_ID]) {
			
				$s_expr		= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID];
				$host		= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_HOST_ID];
				$key		= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_KEY_ID];
				$function	= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_FUNCTION_NAME_ID];
				$parameter	= &$arr[ZBX_EXPRESSION_SIMPLE_EXPRESSION_ID + ZBX_SIMPLE_EXPRESSION_FUNCTION_PARAM_ID];

				$item_res = DBselect('SELECT i.itemid '.
					' FROM items i,hosts h'.
					' WHERE i.key_='.zbx_dbstr($key).
						' AND h.host='.zbx_dbstr($host).
						' AND h.hostid=i.hostid');

				while(($item = DBfetch($item_res)) && (!in_node($item['itemid']))){
				}
				
				if(!$item) return null;

				$item = $item["itemid"];

				$functionid = get_dbid("functions","functionid");

				if ( !DBexecute('insert into functions (functionid,itemid,triggerid,function,parameter)'.
					' values ('.$functionid.','.$item.','.$triggerid.','.zbx_dbstr($function).','.
					zbx_dbstr($parameter).')'))
				{
					return	null;
				}
				$short_exp = str_replace($s_expr,'{'.$functionid.'}',$short_exp);
				$expr = str_replace($s_expr,$ZBX_TR_EXPR_REPLACE_TO,$expr);
				continue;
			}
			$expr = $arr[ZBX_EXPRESSION_LEFT_ID].$ZBX_TR_EXPR_REPLACE_TO.$arr[ZBX_EXPRESSION_RIGHT_ID];
		}

	return $short_exp;
	}

	function update_trigger_comments($triggerids,$comments){
		zbx_value2array($triggerids);
		
		return	DBexecute('UPDATE triggers '.
						' SET comments='.zbx_dbstr($comments).
						' WHERE '.DBcondition('triggerid',$triggerids));
	}

	# Update Trigger status

	function update_trigger_status($triggerids,$status){
		zbx_value2array($triggerids);

		// first update status for child triggers
		$upd_chd_triggers = array();
		$db_chd_triggers = get_triggers_by_templateid($triggerids);
		while($db_chd_trigger = DBfetch($db_chd_triggers)){
			$upd_chd_triggers[$db_chd_trigger['triggerid']] = $db_chd_trigger['triggerid'];
		}
		if(!empty($upd_chd_triggers)){
			update_trigger_status($upd_chd_triggers,$status);
		}

		add_event($triggerids,TRIGGER_VALUE_UNKNOWN);
		
	return	DBexecute('UPDATE triggers SET status='.$status.' WHERE '.DBcondition('triggerid',$triggerids));
	}

	/*
	 * Function: extract_numbers
	 *
	 * Description: 
	 *     Extract from string numbers with prefixes (A-Z)
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function extract_numbers($str){
		$numbers = array();
		while ( ereg(ZBX_EREG_NUMBER.'([[:print:]]*)', $str, $arr) ) {
			$numbers[] = $arr[1];
			$str = $arr[2];
		}
	return $numbers;
	}

	/*
	 * Function: expand_trigger_description_constants
	 *
	 * Description: 
	 *     substitute simple macros in data string with real values
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *           replcae $1-9 macros
	 *
	 */
	function expand_trigger_description_constants($description, $row){
		if($row && isset($row['expression'])){
			$numbers = extract_numbers(ereg_replace('(\{[0-9]+\})', 'function', $row['expression']));
			$description = $row["description"];

			for ( $i = 0; $i < 9; $i++ ){
				$description = str_replace(
									'$'.($i+1),
									isset($numbers[$i])?$numbers[$i]:'', 
									$description
								);
			}
		}

	return $description;
	}
	/*
	 * Function: expand_trigger_description_by_data
	 *
	 * Description: 
	 *     substitute simple macros in data string with real values
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function expand_trigger_description_by_data($row, $flag = ZBX_FLAG_TRIGGER){
		if($row){
			$description = expand_trigger_description_constants($row['description'], $row);

			if(is_null($row['host'])) $row['host'] = '{HOSTNAME}';
			$description = str_replace('{HOSTNAME}', $row['host'],$description);

			if(zbx_strstr($description,'{ITEM.LASTVALUE}')){
			
				$functionid=trigger_get_N_functionid($row['expression'], 1);

				if(isset($functionid)){
					$sql = 'SELECT i.lastvalue, i.value_type, i.itemid '.
							' FROM items i, functions f '.
							' WHERE i.itemid=f.itemid '.
								' AND f.functionid='.$functionid;
					$row2=DBfetch(DBselect($sql));
					if($row2['value_type']!=ITEM_VALUE_TYPE_LOG){
						$description = str_replace('{ITEM.LASTVALUE}', $row2['lastvalue'], $description);
					}
					else{
						$sql = 'SELECT MAX(clock) as max FROM history_log WHERE itemid='.$row2['itemid'];
						$row3=DBfetch(DBselect($sql));
						if($row3 && !is_null($row3['max'])){
							$sql = 'SELECT value FROM history_log WHERE itemid='.$row2['itemid'].' AND clock='.$row3['max'];
							$row4=DBfetch(DBselect($sql));
							$description = str_replace('{ITEM.LASTVALUE}', $row4['value'], $description);
						}
					}
				}
			}

			if(zbx_strstr($description,'{ITEM.VALUE}')){
				$value=($flag==ZBX_FLAG_TRIGGER)?
						trigger_get_func_value($row['expression'],ZBX_FLAG_TRIGGER,1,1):
						trigger_get_func_value($row['expression'],ZBX_FLAG_EVENT,1,$row['clock']);
						
				$description = str_replace('{ITEM.VALUE}',
						$value,
						$description);
			}

			for($i=1; $i<10; $i++){
				if(zbx_strstr($description,'{ITEM.VALUE'.$i.'}')){
					$value=($flag==ZBX_FLAG_TRIGGER)?
							trigger_get_func_value($row['expression'],ZBX_FLAG_TRIGGER,$i,1):
							trigger_get_func_value($row['expression'],ZBX_FLAG_EVENT,$i,$row['clock']);
							
					$description = str_replace('{ITEM.VALUE'.$i.'}',
							$value,
							$description);
				}

			}
		}
		else{
			$description = '*ERROR*';
		}
	return $description;
	}
	
	function expand_trigger_description_simple($triggerid){
		$sql = 'SELECT DISTINCT t.description,h.host,t.expression,t.triggerid '.
				' FROM triggers t '.
					' LEFT JOIN functions f on t.triggerid=f.triggerid '.
					' LEFT JOIN items i on f.itemid=i.itemid '.
					' LEFT JOIN hosts h on i.hostid=h.hostid '.
				' WHERE t.triggerid='.$triggerid;

	return expand_trigger_description_by_data(DBfetch(DBselect($sql)));
	}

	function expand_trigger_description($triggerid){
		$description=expand_trigger_description_simple($triggerid);
		$description=htmlspecialchars($description);

	return $description;
	}

	function update_trigger_value_to_unknown_by_hostid($hostids){
		zbx_value2array($hostids);

		$triggers = array();		
		$result = DBselect('SELECT DISTINCT t.triggerid '.
			' FROM items i,triggers t,functions f '.
			' WHERE f.triggerid=t.triggerid '.
				' AND f.itemid=i.itemid '.
				' AND '.DBcondition('i.hostid',$hostids));

		$now = time();
		while($row=DBfetch($result)){
			$triggers[$row['triggerid']] = $row['triggerid'];
		}
		if(!empty($triggers)){
// returns updated triggers
			$triggers = add_event($triggers,TRIGGER_VALUE_UNKNOWN,$now);
		}

		if(!empty($triggers)){
			DBexecute('UPDATE triggers SET value='.TRIGGER_VALUE_UNKNOWN.', lastchange='.$now.' WHERE '.DBcondition('triggerid',$triggers));
		}
	return true;
	}

/******************************************************************************
 *                                                                            *
 * Comments: !!! Don't forget sync code with C !!!                            *
 *           !!! C code dosn't support TRIGGERS MULTI EVENT !!!				  *
 *                                                                            *
 ******************************************************************************/
	function add_event($triggerids, $value, $time=NULL){
		zbx_value2array($triggerids);
		if(is_null($time)) $time = time();

		$result = DBselect('SELECT DISTINCT triggerid, value, type FROM triggers WHERE '.DBcondition('triggerid',$triggerids));
		while($trigger = DBfetch($result)){
			if(($value == $trigger['value']) && !(($value == TRIGGER_VALUE_TRUE) && ($trigger['type'] == TRIGGER_MULT_EVENT_ENABLED))){
				unset($triggerids[$trigger['triggerid']]);
			}
		}

		$events = array();
		foreach($triggerids as $id => $triggerid){
			$eventid = get_dbid('events','eventid');
			$result = DBexecute('INSERT INTO events (eventid,source,object,objectid,clock,value) '.
					' VALUES ('.$eventid.','.EVENT_SOURCE_TRIGGERS.','.EVENT_OBJECT_TRIGGER.','.$triggerid.','.$time.','.$value.')');
			$events[$eventid] = $eventid;
		}
				
		if(!empty($events) && ($value == TRIGGER_VALUE_FALSE || $value == TRIGGER_VALUE_TRUE)){
			DBexecute('UPDATE alerts '.
						" SET retries=3,error='Trigger changed its status. Will not send repeats.'".
					' WHERE '.DBcondition('eventid',$events).
						' AND repeats>0 '.
						' AND status='.ALERT_STATUS_NOT_SENT);
		}
	return $triggerids;
	}

	function add_trigger_dependency($triggerid,$depid){
		$result = false;
		
		if(check_dependency_by_triggerid($triggerid,$depid)){
			$result=insert_dependency($triggerid,$depid);
		}
		
		//add_additional_dependencies($triggerid,$depid);
	return $result;
	}

/******************************************************************************
 *                                                                            *
 * Purpose: Delete Trigger definition                                         *
 *                                                                            *
 * Comments: !!! Don't forget sync code with C !!!                            *
 *                                                                            *
 ******************************************************************************/
	function delete_trigger($triggerids){
		zbx_value2array($triggerids);

// first delete child triggers
		$del_chd_triggers = array();
		$db_triggers= get_triggers_by_templateid($triggerids);
		while($db_trigger = DBfetch($db_triggers)){// recursion
			$del_chd_triggers[$db_trigger['triggerid']] = $db_trigger['triggerid'];
		}

		if(!empty($del_chd_triggers)){
			$result = delete_trigger($del_chd_triggers);
			if(!$result) return  $result;
		}

// get hosts before functions deletion !!!
		$trig_hosts = array();
		foreach($triggerids as $id => $triggerid){
			$trig_hosts[$triggerid] = get_hosts_by_triggerid($triggerid);
		}

		$result = delete_dependencies_by_triggerid($triggerids);
		if(!$result)	return	$result;

		DBexecute('DELETE FROM trigger_depends WHERE '.DBcondition('triggerid_up',$triggerids));

		$result = delete_function_by_triggerid($triggerids);
		if(!$result)	return	$result;

		$result = delete_events_by_triggerid($triggerids);
		if(!$result)	return	$result;

		$result = delete_services_by_triggerid($triggerids);
		if(!$result)	return	$result;

		$result = delete_sysmaps_elements_with_triggerid($triggerids);
		if(!$result)	return	$result;
		
		DBexecute('DELETE FROM sysmaps_link_triggers WHERE '.DBcondition('triggerid',$triggerids));
		
// disable actions
		$actionids = array();
		$sql = 'SELECT DISTINCT actionid '.
				' FROM conditions '.
				' WHERE conditiontype='.CONDITION_TYPE_TRIGGER.
					' AND '.DBcondition('value',$triggerids,false,true);   // FIXED[POSIBLE value type violation]!!!
		$db_actions = DBselect($sql);
		while($db_action = DBfetch($db_actions)){
			$actionids[$db_action['actionid']] = $db_action['actionid'];
		}

		DBexecute('UPDATE actions '.
					' SET status='.ACTION_STATUS_DISABLED.
					' WHERE '.DBcondition('actionid',$actionids));
							
// delete action conditions	
		DBexecute('DELETE FROM conditions '.
					' WHERE conditiontype='.CONDITION_TYPE_TRIGGER.
						' AND '.DBcondition('value',$triggerids,false,true)); // FIXED[POSIBLE value type violation]!!!

// Get triggers INFO before delete them!
		$triggers = array();
		$trig_res = DBselect('SELECT triggerid, description FROM triggers WHERE '.DBcondition('triggerid',$triggerids));
		while($trig_rows = DBfetch($trig_res)){
			$triggers[$trig_rows['triggerid']] = $trig_rows;
		}
// --

		$result = DBexecute('DELETE FROM triggers WHERE '.DBcondition('triggerid',$triggerids));
		if($result){
			foreach($triggers as $triggerid => $trigger){
				$msg = "Trigger '".$trigger["description"]."' deleted";
				$trig_host = DBfetch($trig_hosts[$triggerid]);
				if($trig_host){
					$msg .= " from host '".$trig_host["host"]."'";
				}
				info($msg);
			}
		}
	return $result;
	}

// Update Trigger definition

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function update_trigger($triggerid,$expression=NULL,$description=NULL,$type=NULL,$priority=NULL,$status=NULL,
		$comments=NULL,$url=NULL,$deps=array(),$templateid=0){
		$trigger	= get_trigger_by_triggerid($triggerid);
		$trig_hosts	= get_hosts_by_triggerid($triggerid);
		$trig_host	= DBfetch($trig_hosts);

		$event_to_unknown = false;		

		if(is_null($expression)){
			/* Restore expression */
			$expression = explode_exp($trigger["expression"],0);
		} 
		else if($expression != explode_exp($trigger["expression"],0)){
			$event_to_unknown = true;
		}
		
		if ( !validate_expression($expression) )
			return false;

		$exp_hosts 	= get_hosts_by_expression($expression);
		
		if( $exp_hosts ){
			$chd_hosts	= get_hosts_by_templateid($trig_host["hostid"]);

			if(DBfetch($chd_hosts)){
				$exp_host = DBfetch($exp_hosts);

				$db_chd_triggers = get_triggers_by_templateid($triggerid);
				while($db_chd_trigger = DBfetch($db_chd_triggers)){
					$chd_trig_hosts = get_hosts_by_triggerid($db_chd_trigger["triggerid"]);
					$chd_trig_host = DBfetch($chd_trig_hosts);

					$newexpression = str_replace(
						"{".$exp_host["host"].":",
						"{".$chd_trig_host["host"].":",
						$expression);

				// recursion
					update_trigger(
						$db_chd_trigger["triggerid"],
						$newexpression,
						$description,
						$type,
						$priority,
						NULL,		// status
						$comments,
						$url,
						replace_template_dependencies($deps, $chd_trig_host['hostid']),
						$triggerid);
				}
			}
		}

		$result=delete_function_by_triggerid($triggerid);
		if(!$result){
			return	$result;
		}

		$expression = implode_exp($expression,$triggerid); /* errors can be ignored cose function must return NULL */

		if($event_to_unknown) add_event($triggerid,TRIGGER_VALUE_UNKNOWN);
		reset_items_nextcheck($triggerid);

		$sql="UPDATE triggers SET";
		if(!is_null($expression))	$sql .= ' expression='.zbx_dbstr($expression).',';
		if(!is_null($description))	$sql .= ' description='.zbx_dbstr($description).',';
		if(!is_null($type))			$sql .= ' type='.$type.',';
		if(!is_null($priority))		$sql .= ' priority='.$priority.',';
		if(!is_null($status))		$sql .= ' status='.$status.',';
		if(!is_null($comments))		$sql .= ' comments='.zbx_dbstr($comments).',';
		if(!is_null($url))			$sql .= ' url='.zbx_dbstr($url).',';
		if(!is_null($templateid))	$sql .= ' templateid='.$templateid.',';
		$sql .= ' value=2 WHERE triggerid='.$triggerid;

		$result = DBexecute($sql);

		delete_dependencies_by_triggerid($triggerid);

		foreach($deps as $id => $triggerid_up){
			if(!$result2=add_trigger_dependency($triggerid, $triggerid_up)){
				error(S_INCORRECT_DEPENDENCY.' ['.expand_trigger_description($triggerid_up).']');
			}
			$result &= $result2;
		}
		
		if($result){
			$trig_hosts	= get_hosts_by_triggerid($triggerid);
			$msg = "Trigger '".$trigger["description"]."' updated";
			$trig_host = DBfetch($trig_hosts);
			if($trig_host){
				$msg .= " for host '".$trig_host["host"]."'";
			}
			info($msg);
		}
	return $result;
	}

	function check_right_on_trigger_by_triggerid($permission,$triggerid){
		$trigger_data = DBfetch(DBselect('select expression from triggers where triggerid='.$triggerid));

		if(!$trigger_data) return false;

		return check_right_on_trigger_by_expression($permission, explode_exp($trigger_data['expression'], 0));
	}

	function check_right_on_trigger_by_expression($permission,$expression){
		global $USER_DETAILS;
		$available_hosts = get_accessible_hosts_by_user($USER_DETAILS, $permission, null, get_current_nodeid(true));

		$db_hosts = get_hosts_by_expression($expression);
		while($host_data = DBfetch($db_hosts)){
			if(!isset($available_hosts[$host_data['hostid']])) return false;
		}

		return true;
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function delete_dependencies_by_triggerid($triggerids){
		zbx_value2array($triggerids);
		
		$db_deps = DBselect('SELECT triggerid_up, triggerid_down '.
						' FROM trigger_depends '.
						' WHERE '.DBcondition('triggerid_down',$triggerids));
						
		while($db_dep = DBfetch($db_deps)){
			DBexecute('UPDATE triggers SET dep_level=dep_level-1 WHERE triggerid='.$db_dep['triggerid_up']);
			DBexecute('DELETE FROM trigger_depends'.
				' WHERE triggerid_up='.$db_dep['triggerid_up'].
					' AND triggerid_down='.$db_dep['triggerid_down']);
		}
	return true;
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function insert_dependency($triggerid_down,$triggerid_up){
	
		$triggerdepid = get_dbid("trigger_depends","triggerdepid");
		$result=DBexecute("insert into trigger_depends (triggerdepid,triggerid_down,triggerid_up)".
			" values ($triggerdepid,$triggerid_down,$triggerid_up)");
		if(!$result){
			return	$result;
		}
	return DBexecute("update triggers set dep_level=dep_level+1 where triggerid=$triggerid_up");
	}


	function check_dependency_by_triggerid($triggerid,$triggerid_up,$level=0){
		if(bccomp($triggerid,$triggerid_up) == 0) return false;
		if($level > 32) return true;
		
		$level++;
		$result = true;
		
		$sql = 'SELECT triggerid_up FROM trigger_depends WHERE triggerid_down='.$triggerid_up;
		$res = DBselect($sql);
		while(($trig = DBfetch($res)) && $result){
			$result &= check_dependency_by_triggerid($triggerid,$trig['triggerid_up'],$level);
		}
		
	return $result;
	}
/* INCORRECT LOGIC: If 1 depends on 2, and 2 depends on 3, then add dependency 1->3
	
	function add_additional_dependencies($triggerid_down,$triggerid_up){
		$result=DBselect('SELECT triggerid_down '.
						' FROM trigger_depends '.
						' WHERE triggerid_up='.$triggerid_down);
			
		while($row=DBfetch($result)){
			insert_dependency($row['triggerid_down'],$triggerid_up);
			add_additional_dependencies($row["triggerid_down"],$triggerid_up);
		}
		
		$result=DBselect("select triggerid_up from trigger_depends where triggerid_down=$triggerid_up");
		
		while($row=DBfetch($result)){
			insert_dependency($triggerid_down,$row["triggerid_up"]);
			add_additional_dependencies($triggerid_down,$row["triggerid_up"]);
		}
	}
//*/

	function delete_function_by_triggerid($triggerids){
		zbx_value2array($triggerids);
	return	DBexecute('DELETE FROM functions WHERE '.DBcondition('triggerid',$triggerids));
	}

	function delete_events_by_triggerid($triggerids){
		zbx_value2array($triggerids);
	return	DBexecute('DELETE FROM events WHERE '.DBcondition('objectid',$triggerids).' AND object='.EVENT_OBJECT_TRIGGER);
	}

	/******************************************************************************
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function delete_triggers_by_itemid($itemids){
		zbx_value2array($itemids);
		
		$del_triggers = array();
		$result=DBselect('SELECT triggerid FROM functions WHERE '.DBcondition('itemid',$itemids));
		while($row=DBfetch($result)){
			$del_triggers[$row['triggerid']] = $row['triggerid'];
		}
		if(!empty($del_triggers)){
			if(!delete_trigger($del_triggers)) return FALSE;
		}

	return TRUE;
	}

	/******************************************************************************
	 *                                                                            *
	 * Purpose: Delete Service definitions by triggerid                           *
	 *                                                                            *
	 * Comments: !!! Don't forget sync code with C !!!                            *
	 *                                                                            *
	 ******************************************************************************/
	function delete_services_by_triggerid($triggerids){
		zbx_value2array($triggerids);
		
		$result = DBselect('SELECT serviceid FROM services WHERE '.DBcondition('triggerid',$triggerids));
		while($row = DBfetch($result)){
			delete_service($row['serviceid']);
		}
	return	TRUE;
	}
	
	/*
	 * Function: cmp_triggers_exressions
	 *
	 * Description: 
	 * 		Warning: function compares ONLY expressions,there is no check on functions and items
	 *     
	 * Author: 
	 *     Aly
	 *
	 * Comments: 
	 *
	 */
	function cmp_triggers_exressions($triggerid1, $triggerid2){
// compare EXPRESSION !!!
		$trig1 = get_trigger_by_triggerid($triggerid1);
		$trig2 = get_trigger_by_triggerid($triggerid2);

		$trig_fnc1 = get_functions_by_triggerid($triggerid1);
		$expr1 = $trig1["expression"];
		while($fnc1 = DBfetch($trig_fnc1)){
			$trig_fnc2 = get_functions_by_triggerid($triggerid2);
			while($fnc2 = DBfetch($trig_fnc2)){
				$expr1 = str_replace(
					"{".$fnc1["functionid"]."}",
					"{".$fnc2["functionid"]."}",
					$expr1);
				break;
			}
		}
	return strcmp($expr1,$trig2["expression"]);
	}

	/*
	 * Function: cmp_triggers
	 *
	 * Description: 
	 *     compare triggers by expression
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function cmp_triggers($triggerid1, $triggerid2){
// compare EXPRESSION !!!
		$trig1 = get_trigger_by_triggerid($triggerid1);
		$trig2 = get_trigger_by_triggerid($triggerid2);

		$trig_fnc1 = get_functions_by_triggerid($triggerid1);
		
		$expr1 = $trig1["expression"];
		while($fnc1 = DBfetch($trig_fnc1)){
			$trig_fnc2 = get_functions_by_triggerid($triggerid2);
			while($fnc2 = DBfetch($trig_fnc2)){
				if(strcmp($fnc1["function"],$fnc2["function"]))	continue;
				if($fnc1["parameter"] != $fnc2["parameter"])	continue;

				$item1 = get_item_by_itemid($fnc1["itemid"]);
				$item2 = get_item_by_itemid($fnc2["itemid"]);

				if(strcmp($item1["key_"],$item2["key_"]))	continue;

				$expr1 = str_replace(
					"{".$fnc1["functionid"]."}",
					"{".$fnc2["functionid"]."}",
					$expr1);
				break;
			}
		}
		return strcmp($expr1,$trig2["expression"]);
	}

	/*
	 * Function: delete_template_triggers
	 *
	 * Description: 
	 *     Delete template triggers
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function delete_template_triggers($hostid, $templateids = null, $unlink_mode = false){
		zbx_value2array($templateids);
		
		$triggers = get_triggers_by_hostid($hostid);
		while($trigger = DBfetch($triggers)){
			if($trigger['templateid']==0)	continue;

			if($templateids != null){				
				$db_tmp_hosts = get_hosts_by_triggerid($trigger["templateid"]);
				$tmp_host = DBfetch($db_tmp_hosts);

				if(!uint_in_array($tmp_host["hostid"], $templateids)) continue;
			}

			if($unlink_mode){
				if(DBexecute('UPDATE triggers SET templateid=0 WHERE triggerid='.$trigger['triggerid'])){
						info('Trigger "'.$trigger["description"].'" unlinked');
				}
			}
			else{
				delete_trigger($trigger["triggerid"]);
			}
		}
	return TRUE;
	}
	
	/*
	 * Function: copy_template_triggers
	 *
	 * Description: 
	 *     Copy triggers from template
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function copy_template_triggers($hostid, $templateid = null, $copy_mode = false){
		if(null == $templateid){
			$templateid = array_keys(get_templates_by_hostid($hostid));
		}

		if(is_array($templateid)){
			foreach($templateid as $id)
				copy_template_triggers($hostid, $id, $copy_mode); // attention recursion
			return;
		}

		$triggers = get_triggers_by_hostid($templateid);
		while($trigger = DBfetch($triggers)){
			copy_trigger_to_host($trigger["triggerid"], $hostid, $copy_mode);
		}

		update_template_dependencies_for_host($hostid);
	}

	/*
	 * Function: update_template_dependencies_for_host
	 *
	 * Description: 
	 *     Update template triggers
	 *     
	 * Author: 
	 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
	 *
	 * Comments: !!! Don't forget sync code with C !!!
	 *
	 */
	function	update_template_dependencies_for_host($hostid){
	
		$db_triggers = get_triggers_by_hostid($hostid);
		
		while($trigger_data = DBfetch($db_triggers)){
			$db_chd_triggers = get_triggers_by_templateid($trigger_data['triggerid']);

			while($chd_trigger_data = DBfetch($db_chd_triggers)){
				update_trigger($chd_trigger_data['triggerid'],
					/*$expression*/		NULL,
					/*$description*/	NULL,
					/*$type*/			NULL,
					/*$priority*/		NULL,
					/*$status*/			NULL,
					/*$comments*/		NULL,
					/*$url*/			NULL,
					replace_template_dependencies(
						get_trigger_dependencies_by_triggerid($trigger_data['triggerid']),
						$hostid),
					$trigger_data['triggerid']);
			}

		}
	}

/*
 * Function: get_triggers_overview
 *
 * Description: 
 *     Retrive table with overview of triggers
 *     
 * Author: 
 *     Eugene Grigorjev (eugene.grigorjev@zabbix.com)
 *
 * Comments: !!! Don't forget sync code with C !!!
 *
 */
	function get_triggers_overview($hostids,$view_style=null){		
		$available_triggers = get_accessible_triggers(PERM_READ_ONLY,$hostids);
		
		if(is_null($view_style)) $view_style = get_profile('web.overview.view.style',STYLE_TOP);
		
		$table = new CTableInfo(S_NO_TRIGGERS_DEFINED);

		$result=DBselect('SELECT DISTINCT t.triggerid,t.description,t.expression,t.value,t.priority,t.lastchange,h.hostid,h.host'.
			' FROM hosts h,items i,triggers t, functions f '.
			' WHERE h.status='.HOST_STATUS_MONITORED.
				' AND h.hostid=i.hostid '.
				' AND i.itemid=f.itemid '.
				' AND f.triggerid=t.triggerid'.
				' AND '.DBcondition('t.triggerid',$available_triggers).
				' AND t.status='.TRIGGER_STATUS_ENABLED.
				' AND i.status='.ITEM_STATUS_ACTIVE.
			' ORDER BY t.description');
		unset($triggers);
		unset($hosts);
		
		$triggers = array();

		while($row = DBfetch($result)){
			if(trigger_dependent($row['triggerid']))	continue;
			
			$row['host'] = get_node_name_by_elid($row['hostid']).$row['host'];
			$row['description'] = expand_trigger_description_constants($row['description'], $row);

			$hosts[strtolower($row['host'])] = $row['host'];

			// A little tricky check for attempt to overwrite active trigger (value=1) with
			// inactive or active trigger with lower priority.
			if(!isset($triggers[$row['description']][$row['host']]) ||
				(
					(($triggers[$row['description']][$row['host']]['value'] == TRIGGER_VALUE_FALSE) && ($row['value'] == TRIGGER_VALUE_TRUE)) ||
					(
						(($triggers[$row['description']][$row['host']]['value'] == TRIGGER_VALUE_FALSE) || ($row['value'] == TRIGGER_VALUE_TRUE)) &&
						($row['priority'] > $triggers[$row['description']][$row['host']]['priority']) 
					)
				)
			)
			{
				$triggers[$row['description']][$row['host']] = array(
					'hostid'	=> $row['hostid'],
					'triggerid'	=> $row['triggerid'],
					'value'		=> $row['value'],
					'lastchange'=> $row['lastchange'],
					'priority'	=> $row['priority']);
			}
		}
		
		if(!isset($hosts)){
			return $table;
		}
		ksort($hosts);
		
		if($view_style == STYLE_TOP){
			$header=array(new CCol(S_TRIGGERS,'center'));
			foreach($hosts as $hostname){
				$header=array_merge($header,array(new CImg('vtext.php?text='.$hostname)));
			}
			$table->setHeader($header,'vertical_header');

			foreach($triggers as $descr => $trhosts){
				$table_row = array(nbsp($descr));
				foreach($hosts as $hostname){
					$table_row=get_trigger_overview_cells($table_row,$trhosts,$hostname);
				}
				$table->AddRow($table_row);
			}
		}
		else{
			$header=array(new CCol(S_HOSTS,'center'));
			foreach($triggers as $descr => $trhosts){
				$descr = array(new CImg('vtext.php?text='.$descr));
				array_push($header,$descr);
			}
			$table->SetHeader($header,'vertical_header');

			foreach($hosts as $hostname){
				$table_row = array(nbsp($hostname));
				foreach($triggers as $descr => $trhosts){
					$table_row=get_trigger_overview_cells($table_row,$trhosts,$hostname);
				}
				$table->AddRow($table_row);
			}
		}
		return $table;
	}

	function get_trigger_overview_cells(&$table_row,&$trhosts,&$hostname){
		$css_class = NULL;

		unset($tr_ov_menu);
		$ack = null;
		if(isset($trhosts[$hostname])){
			unset($ack_menu);
			
			switch($trhosts[$hostname]['value']){
				case TRIGGER_VALUE_TRUE:
					$css_class = get_severity_style($trhosts[$hostname]['priority']);
					if( ($ack = get_last_event_by_triggerid($trhosts[$hostname]['triggerid'])) )
						$ack_menu = array(S_ACKNOWLEDGE, 'acknow.php?eventid='.$ack['eventid'], array('tw'=>'_blank'));

					if ( 1 == $ack['acknowledged'] )
						$ack = new CImg('images/general/tick.png','ack');
					else
						$ack = null;

					break;
				case TRIGGER_VALUE_FALSE:
					$css_class = 'normal';
					break;
				default:
					$css_class = 'unknown_trigger';
			}

			$style = 'cursor: pointer; ';

			if((time(NULL)-$trhosts[$hostname]['lastchange'])<300)
				$style .= 'background-image: url(images/gradients/blink1.gif); '.
					'background-position: top left; '.
					'background-repeat: repeat;';
			else if((time(NULL)-$trhosts[$hostname]['lastchange'])<900)
				$style .= 'background-image: url(images/gradients/blink2.gif); '.
					'background-position: top left; '.
					'background-repeat: repeat;';

			unset($item_menu);
			$tr_ov_menu = array(
				/* name, url, (target [tw], statusbar [sb]), css, submenu */
				array(S_TRIGGER, null,  null, 
					array('outer'=> array('pum_oheader'), 'inner'=>array('pum_iheader'))
					),
				array(S_EVENTS, 'events.php?triggerid='.$trhosts[$hostname]['triggerid'], array('tw'=>'_blank'))
				);

			if(isset($ack_menu)) $tr_ov_menu[] = $ack_menu;

			$db_items = DBselect('select distinct i.itemid, i.description, i.key_, i.value_type '.
				' from items i, functions f '.
				' where f.itemid=i.itemid and f.triggerid='.$trhosts[$hostname]['triggerid']);

			while($item_data = DBfetch($db_items)){
				$description = item_description($item_data);
				switch($item_data['value_type']){
					case ITEM_VALUE_TYPE_UINT64:
					case ITEM_VALUE_TYPE_FLOAT:
						$action = 'showgraph';
						$status_bar = S_SHOW_GRAPH_OF_ITEM.' \''.$description.'\'';
						break;
					case ITEM_VALUE_TYPE_LOG:
					case ITEM_VALUE_TYPE_STR:
					case ITEM_VALUE_TYPE_TEXT:
					default:
						$action = 'showlatest';
						$status_bar = S_SHOW_VALUES_OF_ITEM.' \''.$description.'\'';
						break;
				}
				
				if(strlen($description) > 25) $description = substr($description,0,22).'...';

				$item_menu[$action][] = array(
					$description,
					'history.php?action='.$action.'&itemid='.$item_data['itemid'].'&period=3600',
					 array('tw'=>'', 'sb'=>$status_bar));
			}
			if(isset($item_menu['showgraph'])){
				$tr_ov_menu[] = array(S_GRAPHS,	null, null,
					array('outer'=> array('pum_oheader'), 'inner'=>array('pum_iheader'))
					);
				$tr_ov_menu = array_merge($tr_ov_menu, $item_menu['showgraph']);
			}
			
			if(isset($item_menu['showlatest'])){
				$tr_ov_menu[] = array(S_VALUES,	null, null, 
					array('outer'=> array('pum_oheader'), 'inner'=>array('pum_iheader'))
					);
				$tr_ov_menu = array_merge($tr_ov_menu, $item_menu['showlatest']);
			}

			unset($item_menu);
		}
// dependency
// TRIGGERS ON WHICH DEPENDS THIS
		$desc = array();
		if(isset($trhosts[$hostname])){

			$triggerid = $trhosts[$hostname]['triggerid'];
	
			$dependency = false;
			$dep_table = new CTableInfo();
			$dep_table->AddOption('style', 'width: 200px;');
			$dep_table->addRow(bold(S_DEPENDS_ON.':'));
			
			$sql_dep = 'SELECT * FROM trigger_depends WHERE triggerid_down='.$triggerid;
			$dep_res = DBselect($sql_dep);
			while($dep_row = DBfetch($dep_res)){
				$dep_table->addRow(SPACE.'-'.SPACE.expand_trigger_description($dep_row['triggerid_up']));
				$dependency = true;
			}
			
			if($dependency){
				$img = new Cimg('images/general/down_icon.png','DEP_DOWN');
				$img->AddOption('style','vertical-align: middle; border: 0px;');
				$img->SetHint($dep_table);
				
				array_push($desc,$img);
			}
			unset($img, $dep_table, $dependency);
			
// TRIGGERS THAT DEPEND ON THIS		
			$dependency = false;
			$dep_table = new CTableInfo();
			$dep_table->AddOption('style', 'width: 200px;');
			$dep_table->addRow(bold(S_DEPENDENT.':'));
			
			$sql_dep = 'SELECT * FROM trigger_depends WHERE triggerid_up='.$triggerid;
			$dep_res = DBselect($sql_dep);
			while($dep_row = DBfetch($dep_res)){
				$dep_table->addRow(SPACE.'-'.SPACE.expand_trigger_description($dep_row['triggerid_down']));
				$dependency = true;
			}
			
			if($dependency){
				$img = new Cimg('images/general/up_icon.png','DEP_UP');
				$img->AddOption('style','vertical-align: middle; border: 0px;');
				$img->SetHint($dep_table);
				
				array_push($desc,$img);
			}
			unset($img, $dep_table, $dependency);
		}
//------------------------
		$status_col = new CCol(array($desc, $ack),$css_class);
		if(isset($style)){
			$status_col->AddOption('style', $style);
		}

		if(isset($tr_ov_menu)){
			$tr_ov_menu  = new CPUMenu($tr_ov_menu,170);
			$status_col->OnClick($tr_ov_menu->GetOnActionJS());
			$status_col->AddAction('onmouseover',
				'this.old_border=this.style.border; this.style.border=\'1px dotted #0C0CF0\'');
			$status_col->AddAction('onmouseout', 'this.style.border=this.old_border;');
		}
		array_push($table_row,$status_col);	
	return $table_row;
	}

	function get_function_by_functionid($functionid){
		$result=DBselect('SELECT * FROM functions WHERE functionid='.$functionid);
		$row=DBfetch($result);
		if($row){
			return	$row;
		}
		else{
			error("No function with functionid=[$functionid]");
		}
	return $item;
	}

	function calculate_availability($triggerid,$period_start,$period_end){
		$start_value = -1;
		
		if(($period_start>0) && ($period_start < time())){
			$sql='SELECT eventid, value '.
					' FROM events '.
					' WHERE objectid='.$triggerid.
						' AND object='.EVENT_OBJECT_TRIGGER.
						' AND clock<'.$period_start.
					' ORDER BY eventid DESC';
			if($row = DBfetch(DBselect($sql,1))){
				$start_value = $row['value'];
				$min = $period_start;
			}
		}
//SDI('ST: '.$start_value);
		$sql='SELECT COUNT(*) as cnt, MIN(clock) as minn, MAX(clock) as maxx '.
				' FROM events '.
				' WHERE objectid='.$triggerid.
					' AND object='.EVENT_OBJECT_TRIGGER;

		if($period_start!=0)	$sql .= ' AND clock>='.$period_start;
		if($period_end!=0)		$sql .= ' AND clock<='.$period_end;
//SDI($sql);

		$row=DBfetch(DBselect($sql));
		if($row['cnt']>0){
			if(!isset($min)) $min=$row['minn'];

			$max=$row['maxx'];
		}
		else{
			if(($period_start==0)&&($period_end==0)){
				$max=time();
				$min=$max-24*3600;
			}
			else{
				$ret['true_time']		= 0;
				$ret['false_time']		= 0;
				$ret['unknown_time']	= 0;
				$ret['true']		= (TRIGGER_VALUE_TRUE==$start_value)?100:0;
				$ret['false']		= (TRIGGER_VALUE_FALSE==$start_value)?100:0;
				$ret['unknown']		= (TRIGGER_VALUE_UNKNOWN==$start_value)?100:0;
				return $ret;
			}
		}

		$result=DBselect('SELECT eventid,clock,value '.
						' FROM events '.
						' WHERE objectid='.$triggerid.
							' AND object='.EVENT_OBJECT_TRIGGER.
							' AND clock>='.$min.
							' AND clock<='.$max.
						' ORDER BY eventid ASC');

		$state		= $start_value;//-1;
		$true_time	= 0;
		$false_time	= 0;
		$unknown_time	= 0;
		$time		= $min;

		if(($period_start==0)&&($period_end==0)){
			$max=time();
		}
		$rows=0;
		while($row=DBfetch($result)){
			$clock=$row['clock'];
			$value=$row['value'];

			$diff=$clock-$time;
			$time=$clock;

			if($state==-1){
				$state=$value;
				if($state == 0){
					$false_time+=$diff;
				}
				if($state == 1){
					$true_time+=$diff;
				}
				if($state == 2){
					$unknown_time+=$diff;
				}
			}
			else if($state==0){
				$false_time+=$diff;
				$state=$value;
			}
			else if($state==1){
				$true_time+=$diff;
				$state=$value;
			}
			else if($state==2){
				$unknown_time+=$diff;
				$state=$value;
			}
			$rows++;
		}

		if($rows==0){
			$trigger = get_trigger_by_triggerid($triggerid);
			$state = $trigger['value'];
		}

		if($state==0){
			$false_time=$false_time+$max-$time;
		}
		else if($state==1){
			$true_time=$true_time+$max-$time;
		}
		else if($state==3){
			$unknown_time=$unknown_time+$max-$time;
		}
		$total_time=$true_time+$false_time+$unknown_time;

		if($total_time==0){
			$ret['true_time']	= 0;
			$ret['false_time']	= 0;
			$ret['unknown_time']	= 0;
			$ret['true']		= 0;
			$ret['false']		= 0;
			$ret['unknown']		= 100;
		}
		else{
			$ret['true_time']	= $true_time;
			$ret['false_time']	= $false_time;
			$ret['unknown_time']	= $unknown_time;
			$ret['true']		= (100*$true_time)/$total_time;
			$ret['false']		= (100*$false_time)/$total_time;
			$ret['unknown']		= (100*$unknown_time)/$total_time;
		}
		return $ret;
	}

	/*
	 * Function: trigger_depenent_rec
	 *
	 * Description: 
	 *     check if trigger depends on other triggers having status TRUE
	 *     
	 * Author: 
	 *     Alexei Vladishev
	 *
	 * Comments: Recursive function
	 *
	 */
	function trigger_dependent_rec($triggerid,&$level){
		$ret = FALSE;
	
		$level++;

		/* Check for recursive loop */
		if($level > 32)	return $ret;

		$sql = 'SELECT t.triggerid, t.value '.
				' FROM trigger_depends d, triggers t '.
				' WHERE d.triggerid_down='.$triggerid.
					' AND d.triggerid_up=t.triggerid';

		$result = DBselect($sql);
		while($row = DBfetch($result)){			
			if(TRIGGER_VALUE_TRUE == $row['value'] || trigger_dependent_rec($row['triggerid'], $level)){
				$ret = TRUE;
				break;
			}
		}

	return $ret;
	}

	/*
	 * Function: trigger_depenent 
	 *
	 * Description: 
	 *     check if trigger depends on other triggers having status TRUE
	 *     
	 * Author: 
	 *     Alexei Vladishev
	 *
	 * Comments:
	 *
	 */
	function	trigger_dependent($triggerid){
		$level = 0;
		return trigger_dependent_rec($triggerid, $level);
	}

	/*
	 * Function: trigger_get_N_functionid 
	 *
	 * Description: 
	 *     get functionid of Nth function of trigger expression
	 *     
	 * Author: 
	 *     Alexei Vladishev
	 *
	 * Comments:
	 *
	 */
	function	trigger_get_N_functionid($expression, $function){
		$result = NULL;

		$arr=split('[\{\}]',$expression);
		$num=1;
		foreach($arr as $id){
			if(is_numeric($id))
			{
				if($num == $function){
					$result = $id;
					break;
				}
				$num++;
			}
		}
		return $result;
	}

	/*
	 * Function: trigger_get_func_value 
	 *
	 * Description: 
	 *     get historical value of Nth function of trigger expression
	 *     flag:  ZBX_FLAG_EVENT - get value by clock, ZBX_FLAG_TRIGGR - get value by index
	 *     ZBX_FLAG_TRIGGER, param: 0 - last value, 1 - prev, 2 - prev prev, etc     
	 *     ZBX_FLAG_EVENT, param: event timestamp
	 *     
	 * Author: 
	 *     Alexei Vladishev
	 *
	 * Comments:
	 *
	 */
	function	trigger_get_func_value($expression, $flag, $function, $param){
		$result = NULL;

		$functionid=trigger_get_N_functionid($expression,$function);
		if(isset($functionid)){
			$row=DBfetch(DBselect('select i.* from items i, functions f '.
				' where i.itemid=f.itemid and f.functionid='.$functionid));
			if($row)
			{
				$result=($flag == ZBX_FLAG_TRIGGER)?
					item_get_history($row, $param):
					item_get_history($row, 0, $param);
			}
		}
		return $result;
	}
	
	function get_row_for_nofalseforb($row,$sql){
		$res_events = DBSelect($sql,1);

		if(!$e_row=DBfetch($res_events)){
			return false;
		}
		else{
			$row = array_merge($row,$e_row);
		}

		if(($row['value']!=TRIGGER_VALUE_TRUE) && (!event_initial_time($row))){
			if(!$eventid = first_initial_eventid($row,0)){
				return false;
			}

			$sql = 'SELECT e.eventid, e.value'.
					' FROM events e '.
					' WHERE e.eventid='.$eventid.
						' AND e.acknowledged=0';

			$res_events = DBSelect($sql,1);
			if(!$e_row=DBfetch($res_events)){
				return false;
			}
			else{
				$row = array_merge($row,$e_row);
			}
		}
	return $row;
	}


// author: Aly	
	function make_trigger_details($triggerid,&$trigger_data){
		$table = new CTableInfo();
		
		if(is_show_subnodes()){
			$table->AddRow(array(S_NODE, get_node_name_by_elid($triggerid)));
		}
	
		$table->AddRow(array(S_HOST, $trigger_data['host']));
		$table->AddRow(array(S_TRIGGER, $trigger_data['exp_desc']));
		$table->AddRow(array(S_SEVERITY, new CCol(get_severity_description($trigger_data["priority"]), get_severity_style($trigger_data["priority"]))));
		$table->AddRow(array(S_EXPRESSION, $trigger_data['exp_expr']));
		$table->AddRow(array(S_EVENT_GENERATION, S_NORMAL.((TRIGGER_MULT_EVENT_ENABLED==$trigger_data['type'])?SPACE.'+'.SPACE.S_MULTIPLE_TRUE_EVENTS:'')));
		$table->AddRow(array(S_DISABLED, ((TRIGGER_STATUS_ENABLED==$trigger_data['status'])?new CCol(S_NO,'off'):new CCol(S_YES,'on')) ));
		
	return $table;
	}

?>
