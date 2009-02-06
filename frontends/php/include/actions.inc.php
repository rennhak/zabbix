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

include_once 'include/discovery.inc.php';

?>
<?php
function action_accessible($actionid,$perm){
	global $USER_DETAILS;

	$result = false;

	if (DBselect('select actionid from actions where actionid='.$actionid.' and '.DBin_node('actionid'))){
		$result = true;
		
		$available_hosts = get_accessible_hosts_by_user($USER_DETAILS,$perm,null,get_current_nodeid(true));
		$available_groups = get_accessible_groups_by_user($USER_DETAILS,$perm,null,get_current_nodeid(true));
		
		$db_result = DBselect('SELECT * FROM conditions WHERE actionid='.$actionid);
		while(($ac_data = DBfetch($db_result)) && $result){
			if($ac_data['operator'] != 0) continue;

			switch($ac_data['conditiontype']){
				case CONDITION_TYPE_HOST_GROUP:
					if(!isset($available_groups[$ac_data['value']])){
						$result = false;
					}
					break;
				case CONDITION_TYPE_HOST:
				case CONDITION_TYPE_HOST_TEMPLATE:
					if(!isset($available_hosts[$ac_data['value']])){
						$result = false;
					}
					break;
				case CONDITION_TYPE_TRIGGER:
					$sql = 'SELECT DISTINCT t.triggerid'.
						' FROM triggers t,items i,functions f '.
						' WHERE t.triggerid='.$ac_data['value'].
							' AND f.triggerid=t.triggerid'.
							' AND i.itemid=f.itemid '.
							' AND '.DBcondition('i.hostid',$available_hosts, true);
							
					if(DBfetch(DBselect($sql,1))){
						$result = false;
					}
					break;
			}
		}
	}
	return $result;
}

function check_permission_for_action_conditions($conditions){
	global $USER_DETAILS;

	$result = true;

	$available_hosts = get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,null,get_current_nodeid(true));
	$available_groups = get_accessible_groups_by_user($USER_DETAILS,PERM_READ_ONLY,null,get_current_nodeid(true));

	foreach($conditions as $ac_data){
		if($ac_data['operator'] != 0) continue;

		switch($ac_data['type']){
			case CONDITION_TYPE_HOST_GROUP:
				if(!isset($available_groups[$ac_data['value']])){
					error(S_INCORRECT_GROUP);
					$result = false;
				}
				break;
			case CONDITION_TYPE_HOST:
			case CONDITION_TYPE_HOST_TEMPLATE:
				if(!isset($available_hosts[$ac_data['value']])){
					error(S_INCORRECT_HOST);
					$result = false;
				}
				break;
			case CONDITION_TYPE_TRIGGER:
				$sql = 'SELECT DISTINCT t.triggerid'.
						' FROM triggers t,items i,functions f '. //,events e'.
						' WHERE t.triggerid='.$ac_data['value'].
							' AND f.triggerid=t.triggerid'.
							' AND i.itemid=f.itemid '.
							' AND '.DBcondition('i.hostid',$available_hosts, true);
//								' AND e.eventid='.$ac_data['value'].
//								' AND t.triggerid=e.objectid';
						
				if(DBfetch(DBselect($sql,1))){
					error(S_INCORRECT_TRIGGER);
					$result = false;
				}
				break;
		}
		if(!$result) break;
	}
	return $result;
}

function get_action_by_actionid($actionid){
	$sql="select * from actions where actionid=$actionid"; 
	$result=DBselect($sql);

	if($row=DBfetch($result)){
		return	$row;
	}
	else{
		error("No action with actionid=[$actionid]");
	}
return	$result;
}

function get_operations_by_actionid($actionid){
	$sql='SELECT * FROM operations WHERE actionid='.$actionid; 
	$result=DBselect($sql);

return	$result;
}


// Add Action's condition

function add_action_condition($actionid, $condition){
	$conditionid = get_dbid("conditions","conditionid");

	$result = DBexecute('INSERT INTO conditions (conditionid,actionid,conditiontype,operator,value)'.
		' values ('.$conditionid.','.$actionid.','.
			$condition['type'].','.
			$condition['operator'].','.
			zbx_dbstr($condition['value']).
		')');
	
	if(!$result)
		return $result;

	return $conditionid;
}

function add_action_operation($actionid, $operation){
	$operationid = get_dbid('operations','operationid');
	
	if(!isset($operation['default_msg'])) $operation['default_msg'] = 0;
	if(!isset($operation['opconditions'])) $operation['opconditions'] = array();

	$result = DBexecute('INSERT INTO operations (operationid, actionid, operationtype, object, objectid, shortdata, longdata, esc_period, esc_step_from, esc_step_to, default_msg, evaltype)'.
		' values('.$operationid.','.$actionid.','.
			$operation['operationtype'].','.
			$operation['object'].','.
			$operation['objectid'].','.
			zbx_dbstr($operation['shortdata']).','.
			zbx_dbstr($operation['longdata']).','.
			$operation['esc_period'].','.
			$operation['esc_step_from'].','.
			$operation['esc_step_to'].','.
			$operation['default_msg'].','.
			$operation['evaltype'].
		')');
	if(!$result)
		return $result;

	foreach($operation['opconditions'] as $num => $opcondition){
		$result &= add_operation_condition($operationid, $opcondition);
	}

	return $operationid;
}

// Add operation condition
function add_operation_condition($operationid, $opcondition){
	$opconditionid = get_dbid("opconditions","opconditionid");

	$result = DBexecute('INSERT INTO opconditions (opconditionid,operationid,conditiontype,operator,value)'.
		' values ('.$opconditionid.','.
			$operationid.','.
			$opcondition['conditiontype'].','.
			$opcondition['operator'].','.
			zbx_dbstr($opcondition['value']).
		')');
	
	if(!$result)
		return $result;

return $opconditionid;
}

// Add Action			
function add_action($name, $eventsource, $esc_period, $def_shortdata, $def_longdata, $recovery_msg, $r_shortdata, $r_longdata, $evaltype, $status, $conditions, $operations){
	if(!is_array($conditions) || count($conditions) == 0){
		/*
		error(S_NO_CONDITIONS_DEFINED);
		return false;
		*/
	}
	else{
		if(!check_permission_for_action_conditions($conditions))
			return false;

		foreach($conditions as $condition)
			if( !validate_condition($condition['type'], $condition['value']) ) return false;
	}

	if(!is_array($operations) || count($operations) == 0){
		error(S_NO_OPERATIONS_DEFINED);
		return false;
	}

	foreach($operations as $operation)
		if( !validate_operation($operation) )	return false;

	$actionid=get_dbid('actions','actionid');

	$result = DBexecute('INSERT INTO actions (actionid,name,eventsource,esc_period,def_shortdata,def_longdata,recovery_msg,r_shortdata,r_longdata,evaltype,status)'.
				' VALUES ('.$actionid.','.zbx_dbstr($name).','.$eventsource.','.$esc_period.','.zbx_dbstr($def_shortdata).','.zbx_dbstr($def_longdata).','.$recovery_msg.','.zbx_dbstr($r_shortdata).','.zbx_dbstr($r_longdata).','.$evaltype.','.$status.')');

	if(!$result)
		return $result;

	foreach($operations as $operation)
		if(!$result = add_action_operation($actionid, $operation))
			break;

	if($result){
		foreach($conditions as $condition)
		if(!$result = add_action_condition($actionid, $condition))
			break;
	}

return $actionid;
}

# Update Action

function update_action($actionid, $name, $eventsource, $esc_period, $def_shortdata, $def_longdata, $recovery_msg, $r_shortdata, $r_longdata, $evaltype, $status, $conditions, $operations){

	if(!is_array($conditions) || count($conditions) == 0){
		/*
		error(S_NO_CONDITIONS_DEFINED);
		return false;
		*/
	}
	else{
		if(!check_permission_for_action_conditions($conditions))
			return false;

		foreach($conditions as $condition)
			if( !validate_condition($condition['type'],$condition['value']) ) return false;
	}

	if(!is_array($operations) || count($operations) == 0){
		error(S_NO_OPERATIONS_DEFINED);
		return false;
	}

	foreach($operations as $operation)
		if( !validate_operation($operation) )	return false;

	$result = DBexecute('UPDATE actions SET name='.zbx_dbstr($name).
							',eventsource='.$eventsource.
							',esc_period='.$esc_period.
							',def_shortdata='.zbx_dbstr($def_shortdata).
							',def_longdata='.zbx_dbstr($def_longdata).
							',recovery_msg='.$recovery_msg.
							',r_shortdata='.zbx_dbstr($r_shortdata).
							',r_longdata='.zbx_dbstr($r_longdata).
							',evaltype='.$evaltype.
							',status='.$status.
						' WHERE actionid='.$actionid);

	if($result){
		DBexecute('DELETE FROM conditions WHERE actionid='.$actionid);

		$opers = get_operations_by_actionid($actionid);
		while($operation = DBFetch($opers)){
			DBexecute('DELETE FROM opconditions WHERE operationid='.$operation['operationid']);
		}
		DBexecute('DELETE FROM operations WHERE actionid='.$actionid);
		

		foreach($operations as $operation)
			if(!$result = add_action_operation($actionid, $operation))
				break;

		if($result){
			foreach($conditions as $condition)
			if(!$result = add_action_condition($actionid, $condition))
				break;
		}		
	}

return $result;
}

# Delete Action

function delete_action( $actionid ){
	$return = DBexecute('delete from conditions where actionid='.$actionid);

	$opers = get_operations_by_actionid($actionid);
	while($operation = DBFetch($opers)){
		DBexecute('DELETE FROM opconditions WHERE operationid='.$operation['operationid']);
	}
	
	if($return)
		$result = DBexecute('delete from operations where actionid='.$actionid);

	if($return)
		$result = DBexecute('delete from alerts where actionid='.$actionid);

	if($return)
		$result = DBexecute('delete from actions where actionid='.$actionid);

	return $result;
}

function	condition_operator2str($operator)
{
	$str_op[CONDITION_OPERATOR_EQUAL] 	= '=';
	$str_op[CONDITION_OPERATOR_NOT_EQUAL]	= '<>';
	$str_op[CONDITION_OPERATOR_LIKE]	= S_LIKE_SMALL;
	$str_op[CONDITION_OPERATOR_NOT_LIKE]	= S_NOT_LIKE_SMALL;
	$str_op[CONDITION_OPERATOR_IN]		= S_IN_SMALL;
	$str_op[CONDITION_OPERATOR_MORE_EQUAL]	= '>=';
	$str_op[CONDITION_OPERATOR_LESS_EQUAL]	= '<=';
	$str_op[CONDITION_OPERATOR_NOT_IN]	= S_NOT_IN_SMALL;

	if(isset($str_op[$operator]))
		return $str_op[$operator];

	return S_UNKNOWN;
}

function	condition_type2str($conditiontype)
{
	$str_type[CONDITION_TYPE_HOST_GROUP]		= S_HOST_GROUP;
	$str_type[CONDITION_TYPE_HOST_TEMPLATE]		= S_HOST_TEMPLATE;
	$str_type[CONDITION_TYPE_TRIGGER]		= S_TRIGGER;
	$str_type[CONDITION_TYPE_HOST]			= S_HOST;
	$str_type[CONDITION_TYPE_TRIGGER_NAME]		= S_TRIGGER_DESCRIPTION;
	$str_type[CONDITION_TYPE_TRIGGER_VALUE]		= S_TRIGGER_VALUE;
	$str_type[CONDITION_TYPE_TRIGGER_SEVERITY]	= S_TRIGGER_SEVERITY;
	$str_type[CONDITION_TYPE_TIME_PERIOD]		= S_TIME_PERIOD;
	$str_type[CONDITION_TYPE_DHOST_IP]		= S_HOST_IP;
	$str_type[CONDITION_TYPE_DSERVICE_TYPE]		= S_SERVICE_TYPE;
	$str_type[CONDITION_TYPE_DSERVICE_PORT]		= S_SERVICE_PORT;
	$str_type[CONDITION_TYPE_DSTATUS]		= S_DISCOVERY_STATUS;
	$str_type[CONDITION_TYPE_DUPTIME]		= S_UPTIME_DOWNTIME;
	$str_type[CONDITION_TYPE_DVALUE]		= S_RECEIVED_VALUE;
	$str_type[CONDITION_TYPE_EVENT_ACKNOWLEDGED]	= S_EVENT_ACKNOWLEDGED;
	$str_type[CONDITION_TYPE_APPLICATION]		= S_APPLICATION;

	if(isset($str_type[$conditiontype]))
		return $str_type[$conditiontype];

	return S_UNKNOWN;
}
	
function condition_value2str($conditiontype, $value){
	switch($conditiontype){
		case CONDITION_TYPE_HOST_GROUP:
			$group = get_hostgroup_by_groupid($value);
			
			$str_val = '';
			if(id2nodeid($value) != get_current_nodeid()) $str_val = get_node_name_by_elid($value, true);
			$str_val.= $group['name'];
			break;
		case CONDITION_TYPE_TRIGGER:
			$str_val = expand_trigger_description($value);
			break;
		case CONDITION_TYPE_HOST:
		case CONDITION_TYPE_HOST_TEMPLATE:
			$host = get_host_by_hostid($value);			
			$str_val = '';
			if(id2nodeid($value) != get_current_nodeid()) $str_val = get_node_name_by_elid($value, true);
			$str_val.= $host['host'];
			break;
		case CONDITION_TYPE_TRIGGER_NAME:
			$str_val = $value;
			break;
		case CONDITION_TYPE_TRIGGER_VALUE:
			$str_val = trigger_value2str($value);
			break;
		case CONDITION_TYPE_TRIGGER_SEVERITY:
			$str_val = get_severity_description($value);
			break;
		case CONDITION_TYPE_TIME_PERIOD:
			$str_val = $value;
			break;
		case CONDITION_TYPE_DHOST_IP:
			$str_val = $value;
			break;
		case CONDITION_TYPE_DSERVICE_TYPE:
			$str_val = discovery_check_type2str($value);
			break;
		case CONDITION_TYPE_DSERVICE_PORT:
			$str_val = $value;
			break;
		case CONDITION_TYPE_DSTATUS:
			$str_val = discovery_object_status2str($value);
			break;
		case CONDITION_TYPE_DUPTIME:
			$str_val = $value;
			break;
		case CONDITION_TYPE_DVALUE:
			$str_val = $value;
			break;
		case CONDITION_TYPE_EVENT_ACKNOWLEDGED:
			$str_val = ($value)?S_ACK:S_NOT_ACK;
			break;
		case CONDITION_TYPE_APPLICATION:
			$str_val = $value;
			break;
		default:
			return S_UNKNOWN;
			break;
	}
	return '"'.$str_val.'"';
}

function get_condition_desc($conditiontype, $operator, $value){
	return condition_type2str($conditiontype).' '.
		condition_operator2str($operator).' '.
		condition_value2str($conditiontype, $value);
}

define('LONG_DESCRITION', 0);
define('SHORT_DESCRITION', 1);

function get_operation_desc($type=SHORT_DESCRITION, $data){
	$result = null;

	switch($type){
		case SHORT_DESCRITION:
			switch($data['operationtype']){
				case OPERATION_TYPE_MESSAGE:
					switch($data['object']){
						case OPERATION_OBJECT_USER:
							$obj_data = get_user_by_userid($data['objectid']);
							$obj_data = S_USER.' "'.$obj_data['alias'].'"';
							break;
						case OPERATION_OBJECT_GROUP:
							$obj_data = get_group_by_usrgrpid($data['objectid']);
							$obj_data = S_GROUP.' "'.$obj_data['name'].'"';
							break;
					}
					$result = S_SEND_MESSAGE_TO.' '.$obj_data;
					break;
				case OPERATION_TYPE_COMMAND:
					$result = S_RUN_REMOTE_COMMANDS;
					break;
				case OPERATION_TYPE_HOST_ADD:
					$result = S_ADD_HOST;
					break;
				case OPERATION_TYPE_HOST_REMOVE:
					$result = S_REMOVE_HOST;
					break;
				case OPERATION_TYPE_GROUP_ADD:
					$obj_data = get_hostgroup_by_groupid($data['objectid']);
					$result = S_ADD_TO_GROUP.' "'.$obj_data['name'].'"';
					break;
				case OPERATION_TYPE_GROUP_REMOVE:
					$obj_data = get_hostgroup_by_groupid($data['objectid']);
					$result = S_DELETE_FROM_GROUP.' "'.$obj_data['name'].'"';
					break;
				case OPERATION_TYPE_TEMPLATE_ADD:
					$obj_data = get_host_by_hostid($data['objectid']);
					$result = S_LINK_TO_TEMPLATE.' "'.$obj_data['host'].'"';
					break;
				case OPERATION_TYPE_TEMPLATE_REMOVE:
					$obj_data = get_host_by_hostid($data['objectid']);
					$result = S_UNLINK_FROM_TEMPLATE.' "'.$obj_data['host'].'"';
					break;
				default: break;
			}
			break;
		case LONG_DESCRITION:
			switch($data['operationtype']){
				case OPERATION_TYPE_MESSAGE:
					// for PHP4
					if(isset($data['default_msg']) && !empty($data['default_msg'])){
						if(isset($_REQUEST['def_shortdata']) && isset($_REQUEST['def_longdata'])){
							$temp = bold(S_SUBJECT.': ');
							$result = $temp->ToString().$_REQUEST['def_shortdata']."\n";
							$temp = bold(S_MESSAGE.':');
							$result .= $temp->ToString().$_REQUEST['def_longdata'];
						}
						else if(isset($data['operationid'])){ 
							$sql = 'SELECT a.def_shortdata,a.def_longdata '.
									' FROM actions a, operations o '.
									' WHERE a.actionid=o.actionid '.
										' AND o.operationid='.$data['operationid'];
							if($rows = DBfetch(DBselect($sql,1))){
								$temp = bold(S_SUBJECT.': ');
								$result = $temp->ToString().$rows['def_shortdata']."\n";
								$temp = bold(S_MESSAGE.':');
								$result .= $temp->ToString().$rows['def_longdata'];
							}
						}
					}
					else{
						$temp = bold(S_SUBJECT.': ');
						$result = $temp->ToString().$data['shortdata']."\n";
						$temp = bold(S_MESSAGE.':');
						$result .= $temp->ToString().$data['longdata'];
					}

					break;
				case OPERATION_TYPE_COMMAND:
					$temp = bold(S_REMOTE_COMMANDS.': ');
					$result = $temp->ToString().$data['longdata'];
					break;
				default: break;
			}
			break;
		default:
			break;
	}

	return $result;
}

function get_conditions_by_eventsource($eventsource){
	$conditions[EVENT_SOURCE_TRIGGERS] = array(
			CONDITION_TYPE_APPLICATION,
//			CONDITION_TYPE_EVENT_ACKNOWLEDGED,
			CONDITION_TYPE_HOST_GROUP,
			CONDITION_TYPE_HOST_TEMPLATE,
			CONDITION_TYPE_HOST,
			CONDITION_TYPE_TRIGGER,
			CONDITION_TYPE_TRIGGER_NAME,
			CONDITION_TYPE_TRIGGER_SEVERITY,
			CONDITION_TYPE_TRIGGER_VALUE,
			CONDITION_TYPE_TIME_PERIOD
		);
	$conditions[EVENT_SOURCE_DISCOVERY] = array(
			CONDITION_TYPE_DHOST_IP,
			CONDITION_TYPE_DSERVICE_TYPE,
			CONDITION_TYPE_DSERVICE_PORT,
			CONDITION_TYPE_DSTATUS,
			CONDITION_TYPE_DUPTIME,
			CONDITION_TYPE_DVALUE
		);

	if(isset($conditions[$eventsource]))
		return $conditions[$eventsource];

	return $conditions[EVENT_SOURCE_TRIGGERS];
}

function get_opconditions_by_eventsource($eventsource){
	$conditions[EVENT_SOURCE_TRIGGERS] = array(
			CONDITION_TYPE_EVENT_ACKNOWLEDGED
		);
		
	$conditions[EVENT_SOURCE_DISCOVERY] = array(
		);

	if(isset($conditions[$eventsource]))
		return $conditions[$eventsource];

	return $conditions[EVENT_SOURCE_TRIGGERS];
}

function get_operations_by_eventsource($eventsource){
	$operations[EVENT_SOURCE_TRIGGERS] = array(
			OPERATION_TYPE_MESSAGE,
			OPERATION_TYPE_COMMAND
		);
	$operations[EVENT_SOURCE_DISCOVERY] = array(
			OPERATION_TYPE_MESSAGE,
			OPERATION_TYPE_COMMAND,
			OPERATION_TYPE_HOST_ADD,
			OPERATION_TYPE_HOST_REMOVE,
			OPERATION_TYPE_GROUP_ADD,
			OPERATION_TYPE_GROUP_REMOVE,
			OPERATION_TYPE_TEMPLATE_ADD,
			OPERATION_TYPE_TEMPLATE_REMOVE
		);

	if(isset($operations[$eventsource]))
		return $operations[$eventsource];

	return $operations[EVENT_SOURCE_TRIGGERS];
}

function	operation_type2str($type)
{
	$str_type[OPERATION_TYPE_MESSAGE]		= S_SEND_MESSAGE;
	$str_type[OPERATION_TYPE_COMMAND]		= S_REMOTE_COMMAND;
	$str_type[OPERATION_TYPE_HOST_ADD]		= S_ADD_HOST;
	$str_type[OPERATION_TYPE_HOST_REMOVE]		= S_REMOVE_HOST;
	$str_type[OPERATION_TYPE_GROUP_ADD]		= S_ADD_TO_GROUP;
	$str_type[OPERATION_TYPE_GROUP_REMOVE]		= S_DELETE_FROM_GROUP;
	$str_type[OPERATION_TYPE_TEMPLATE_ADD]		= S_LINK_TO_TEMPLATE;
	$str_type[OPERATION_TYPE_TEMPLATE_REMOVE]	= S_UNLINK_FROM_TEMPLATE;

	if(isset($str_type[$type]))
		return $str_type[$type];

	return S_UNKNOWN;
}

function	get_operators_by_conditiontype($conditiontype)
{
	$operators[CONDITION_TYPE_HOST_GROUP] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_HOST_TEMPLATE] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_HOST] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_TRIGGER] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_TRIGGER_NAME] = array(
			CONDITION_OPERATOR_LIKE,
			CONDITION_OPERATOR_NOT_LIKE	
		);
	$operators[CONDITION_TYPE_TRIGGER_SEVERITY] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL,
			CONDITION_OPERATOR_MORE_EQUAL,
			CONDITION_OPERATOR_LESS_EQUAL
		);
	$operators[CONDITION_TYPE_TRIGGER_VALUE] = array(
			CONDITION_OPERATOR_EQUAL
		);
	$operators[CONDITION_TYPE_TIME_PERIOD] = array(
			CONDITION_OPERATOR_IN,
			CONDITION_OPERATOR_NOT_IN
		);
	$operators[CONDITION_TYPE_DHOST_IP] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_DSERVICE_TYPE] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_DSERVICE_PORT] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL
		);
	$operators[CONDITION_TYPE_DSTATUS] = array(
			CONDITION_OPERATOR_EQUAL,
		);
	$operators[CONDITION_TYPE_DUPTIME] = array(
			CONDITION_OPERATOR_MORE_EQUAL,
			CONDITION_OPERATOR_LESS_EQUAL
		);
	$operators[CONDITION_TYPE_DVALUE] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_NOT_EQUAL,
			CONDITION_OPERATOR_MORE_EQUAL,
			CONDITION_OPERATOR_LESS_EQUAL,
			CONDITION_OPERATOR_LIKE,
			CONDITION_OPERATOR_NOT_LIKE
		);
	$operators[CONDITION_TYPE_EVENT_ACKNOWLEDGED] = array(
			CONDITION_OPERATOR_EQUAL
		);
	$operators[CONDITION_TYPE_APPLICATION] = array(
			CONDITION_OPERATOR_EQUAL,
			CONDITION_OPERATOR_LIKE,
			CONDITION_OPERATOR_NOT_LIKE	
		);

	if(isset($operators[$conditiontype]))
		return $operators[$conditiontype];

	return array();
}

function	update_action_status($actionid, $status)
{
	return DBexecute("update actions set status=$status where actionid=$actionid");
}

function validate_condition($conditiontype, $value){
	global $USER_DETAILS;

	switch($conditiontype){
		case CONDITION_TYPE_HOST_GROUP:
			$available_groups = get_accessible_groups_by_user($USER_DETAILS,PERM_READ_ONLY,null,get_current_nodeid(true));
			if(!isset($available_groups[$value])){
				error(S_INCORRECT_GROUP);
				return false;
			}
			break;
		case CONDITION_TYPE_HOST_TEMPLATE:
			$available_hosts = get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,null,get_current_nodeid(true));
			if(!isset($available_hosts[$value])){
				error(S_INCORRECT_HOST);
				return false;
			}
			break;
		case CONDITION_TYPE_TRIGGER:
			if( !DBfetch(DBselect('select triggerid from triggers where triggerid='.$value)) || 
				!check_right_on_trigger_by_triggerid(PERM_READ_ONLY, $value) )
			{
				error(S_INCORRECT_TRIGGER);
				return false;
			}
			break;
		case CONDITION_TYPE_HOST:
			$available_hosts = get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,null,get_current_nodeid(true));
			if(!isset($available_hosts[$value])){
				error(S_INCORRECT_HOST);
				return false;
			}
			break;
		case CONDITION_TYPE_TIME_PERIOD:
			if( !validate_period($value) ){
				error(S_INCORRECT_PERIOD.' ['.$value.']');
				return false;
			}
			break;
		case CONDITION_TYPE_DHOST_IP:
			if( !validate_ip_range($value) ){
				error(S_INCORRECT_IP.' ['.$value.']');
				return false;
			}
			break;
		case CONDITION_TYPE_DSERVICE_TYPE:
			if( S_UNKNOWN == discovery_check_type2str($value) ){
				error(S_INCORRECT_DISCOVERY_CHECK);
				return false;
			}
			break;
		case CONDITION_TYPE_DSERVICE_PORT:
			if( !validate_port_list($value) ){
				error(S_INCORRECT_PORT.' ['.$value.']');
				return false;
			}
			break;
		case CONDITION_TYPE_DSTATUS:
			if( S_UNKNOWN == discovery_object_status2str($value) ){
				error(S_INCORRECT_DISCOVERY_STATUS);
				return false;
			}
			break;
		case CONDITION_TYPE_EVENT_ACKNOWLEDGED:
			if(S_UNKNOWN == condition_value2str($conditiontype,$value)){
				error(S_INCORRECT_DISCOVERY_STATUS);
				return false;
			}
			break;
		
		case CONDITION_TYPE_TRIGGER_NAME:
		case CONDITION_TYPE_TRIGGER_VALUE:
		case CONDITION_TYPE_TRIGGER_SEVERITY:
		case CONDITION_TYPE_DUPTIME:
		case CONDITION_TYPE_DVALUE:
		case CONDITION_TYPE_APPLICATION:
			break;
		default:
			error(S_INCORRECT_CONDITION_TYPE);
			return false;
			break;
	}
	return true;
}

function validate_operation($operation){
	global $USER_DETAILS;

	switch($operation['operationtype']){
		case OPERATION_TYPE_MESSAGE:
			switch($operation['object']){
				case OPERATION_OBJECT_USER:
					if( !get_user_by_userid($operation['objectid']) ){
						error(S_INCORRECT_USER);
						return false;
					}
					break;
				case OPERATION_OBJECT_GROUP:
					if( !get_group_by_usrgrpid($operation['objectid']) ){
						error(S_INCORRECT_GROUP);
						return false;
					}
					break;
				default:
					error(S_INCORRECT_OBJECT_TYPE);
					return false;
			}
			break;
		case OPERATION_TYPE_COMMAND:
			return validate_commands($operation['longdata']);
		case OPERATION_TYPE_HOST_ADD:
		case OPERATION_TYPE_HOST_REMOVE:
			break;
		case OPERATION_TYPE_GROUP_ADD:
		case OPERATION_TYPE_GROUP_REMOVE:
			if(!uint_in_array($operation['objectid'], get_accessible_groups_by_user($USER_DETAILS,PERM_READ_WRITE,PERM_RES_IDS_ARRAY))){
				error(S_INCORRECT_GROUP);
				return false;
			}
			break;
		case OPERATION_TYPE_TEMPLATE_ADD:
		case OPERATION_TYPE_TEMPLATE_REMOVE:
			if(!uint_in_array($operation['objectid'], get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_WRITE,PERM_RES_IDS_ARRAY))){
				error(S_INCORRECT_HOST);
				return false;
			}
			break;
		default:
			error(S_INCORRECT_OPERATION_TYPE);
			return false;
	}
return true;
}

function validate_commands($commands){
	$cmd_list = split("\n",$commands);
	foreach($cmd_list as $cmd){
		$cmd = trim($cmd, "\x00..\x1F");
		if(!ereg("^(({HOSTNAME})|".ZBX_EREG_INTERNAL_NAMES.")(:|#)[[:print:]]*$",$cmd,$cmd_items)){
			error("Incorrect command: '$cmd'");
			return FALSE;
		}
		
		if($cmd_items[4] == "#"){ // group
			if(!DBfetch(DBselect("select groupid from groups where name=".zbx_dbstr($cmd_items[1])))){
				error("Unknown group name: '".$cmd_items[1]."' in command ".$cmd."'");
				return FALSE;
			}
		}
		else if($cmd_items[4] == ":"){ // host
			if(($cmd_items[1] != '{HOSTNAME}') && !DBfetch(DBselect("select hostid from hosts where host=".zbx_dbstr($cmd_items[1])))){
				error("Unknown host name '".$cmd_items[1]."' in command '".$cmd."'");
				return FALSE;
			}
		}
	}
	return TRUE;
}

function count_operations_delay($operations, $def_period=0){
	$delays = array(0,0);
	$periods = array();
	$max_step = 0;
	foreach($operations as $num => $operation){
		$step_from = $operation['esc_step_from']?$operation['esc_step_from']:1;
		$step_to = $operation['esc_step_to']?$operation['esc_step_to']:9999;
		$esc_period = $operation['esc_period']?$operation['esc_period']:$def_period;
		
		$max_step = ($max_step>$step_from)?$max_step:$step_from;
		
		for($i=$step_from; $i<$step_to; $i++){
			if(isset($periods[$i]) && ($periods[$i] < $esc_period)){
			}
			else{
				$periods[$i]= $esc_period;
			}
		}
	}

	for($i=1; $i<=$max_step; $i++){
		$esc_period = isset($periods[$i])?$periods[$i]:$def_period;
		$delays[$i+1] = $delays[$i] + $esc_period;
	}
	
return $delays;
}

function get_history_of_actions($start,$num,$sql_cond=''){
	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, PERM_RES_IDS_ARRAY);

	$table = new CTableInfo(S_NO_ACTIONS_FOUND);
	$table->SetHeader(array(
			is_show_subnodes() ? make_sorting_link(S_NODES,'a.alertid') : null,
			make_sorting_link(S_TIME,'a.clock'),
			make_sorting_link(S_TYPE,'mt.description'),
			make_sorting_link(S_STATUS,'a.status'),
			make_sorting_link(S_RETRIES_LEFT,'a.retries'),
			make_sorting_link(S_RECIPIENTS,'a.sendto'),
			S_MESSAGE,
			S_ERROR
			));
			
	$sql = 'SELECT DISTINCT a.alertid,a.clock,mt.description,a.sendto,a.subject,a.message,a.status,a.retries,a.error '.
			' FROM events e,alerts a'.
			' left join media_type mt on mt.mediatypeid=a.mediatypeid'.
			' WHERE e.eventid = a.eventid and alerttype in ('.ALERT_TYPE_MESSAGE.')'.
				$sql_cond.
				' AND '.DBcondition('e.objectid',$available_triggers).
				' AND '.DBin_node('a.alertid').
			order_by('a.clock,a.alertid,mt.description,a.sendto,a.status,a.retries');

	$result=DBselect($sql,10*$start+$num);
		
	$col=0;
	$skip=$start;
	while(($row=DBfetch($result))&&($col<$num)){
		if($skip > 0) {
			$skip--;
			continue;
		}
		$time=date("Y.M.d H:i:s",$row["clock"]);

		if($row["status"] == ALERT_STATUS_SENT){
			$status=new CSpan(S_SENT,"green");
			$retries=new CSpan(SPACE,"green");
		}
		else if($row["status"] == ALERT_STATUS_NOT_SENT){
			$status=new CSpan(S_IN_PROGRESS,"orange");
			$retries=new CSpan(ALERT_MAX_RETRIES - $row["retries"],"orange");
		}
		else{
			$status=new CSpan(S_NOT_SENT,"red");
			$retries=new CSpan(0,"red");
		}
		$sendto=$row["sendto"];

		$message = array(bold(S_SUBJECT.': '),br(),$row["subject"],br(),br(),bold(S_MESSAGE.': '),br(),$row['message']);

		if(empty($row["error"])){
			$error=new CSpan(SPACE,"off");
		}
		else{
			$error=new CSpan($row["error"],"on");
		}
		$table->AddRow(array(
			get_node_name_by_elid($row['alertid']),
			new CCol($time, 'top'),
			new CCol($row["description"], 'top'),
			new CCol($status, 'top'),
			new CCol($retries, 'top'),
			new CCol($sendto, 'top'),
			new CCol($message, 'top'),
			new CCol($error, 'wraptext top')));
		$col++;
	}

	return $table;
}
	
// Author: Aly
function get_action_msgs_for_event($eventid){
	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, PERM_RES_IDS_ARRAY);

	$table = new CTableInfo(S_NO_ACTIONS_FOUND);
	$table->SetHeader(array(
			is_show_subnodes() ? make_sorting_link(S_NODES,'a.alertid') : null,
			make_sorting_link(S_TIME,'a.clock'),
			make_sorting_link(S_TYPE,'mt.description'),
			make_sorting_link(S_STATUS,'a.status'),
			make_sorting_link(S_RETRIES_LEFT,'a.retries'),
			make_sorting_link(S_RECIPIENTS,'a.sendto'),
			S_MESSAGE,
			S_ERROR
			));
			
	$sql = 'SELECT DISTINCT a.alertid,a.clock,a.esc_step, mt.description,a.sendto,a.subject,a.message,a.status,a.retries,a.error '.
			' FROM events e,alerts a'.
				' left join media_type mt on mt.mediatypeid=a.mediatypeid'.
			' WHERE a.eventid='.$eventid.
				' AND a.alerttype='.ALERT_TYPE_MESSAGE.
				' AND e.eventid = a.eventid'.
				' AND '.DBcondition('e.objectid',$available_triggers).
				' AND '.DBin_node('a.alertid').
			order_by('a.clock,a.alertid,mt.description,a.sendto,a.status,a.retries');
	$result=DBselect($sql);

	while($row=DBfetch($result)){
		$time=date("Y.M.d H:i:s",$row["clock"]);
		if($row['esc_step'] > 0){
			$time = array(bold(S_STEP.': '),$row["esc_step"],br(),bold(S_TIME.': '),br(),$time);	
		}
		
		if($row["status"] == ALERT_STATUS_SENT){
			$status=new CSpan(S_SENT,"green");
			$retries=new CSpan(SPACE,"green");
		}
		else if($row["status"] == ALERT_STATUS_NOT_SENT){
			$status=new CSpan(S_IN_PROGRESS,"orange");
			$retries=new CSpan(ALERT_MAX_RETRIES - $row["retries"],"orange");
		}
		else{
			$status=new CSpan(S_NOT_SENT,"red");
			$retries=new CSpan(0,"red");
		}
		$sendto=$row["sendto"];

		$message = array(bold(S_SUBJECT.':'),br(),$row["subject"],br(),br(),bold(S_MESSAGE.':'));
		$msg = explode("\n",$row['message']);
		foreach($msg as $m)
		{
			array_push($message, BR(), $m);
		}
		
		if(empty($row["error"])){
			$error=new CSpan(SPACE,"off");
		}
		else{
			$error=new CSpan($row["error"],"on");
		}
		
		$table->AddRow(array(
			get_node_name_by_elid($row['alertid']),
			new CCol($time, 'top'),
			new CCol($row["description"], 'top'),
			new CCol($status, 'top'),
			new CCol($retries, 'top'),
			new CCol($sendto, 'top'),
			new CCol($message, 'wraptext top'),
			new CCol($error, 'wraptext top')));
	}

return $table;
}

// Author: Aly
function get_action_cmds_for_event($eventid){
	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, PERM_RES_IDS_ARRAY);

	$table = new CTableInfo(S_NO_ACTIONS_FOUND);
	$table->SetHeader(array(
			is_show_subnodes()?S_NODES:null,
			S_TIME,
			S_STATUS,
			S_COMMAND,
			S_ERROR
			));
			
	$sql = 'SELECT DISTINCT a.alertid,a.clock,a.sendto,a.esc_step,a.subject,a.message,a.status,a.retries,a.error '.
			' FROM events e,alerts a'.
			' WHERE a.eventid='.$eventid.
				' AND a.alerttype='.ALERT_TYPE_COMMAND.
				' AND e.eventid = a.eventid'.
				' AND '.DBcondition('e.objectid',$available_triggers).
				' AND '.DBin_node('a.alertid').
			order_by('a.clock,a.alertid,a.sendto,a.status,a.retries');
	$result=DBselect($sql);

	while($row=DBfetch($result)){
		$time=date("Y.M.d H:i:s",$row["clock"]);
		if($row['esc_step'] > 0){
			$time = array(bold(S_STEP.': '),$row["esc_step"],br(),bold(S_TIME.': '),br(),$time);	
		}
		
		if($row["status"] == ALERT_STATUS_SENT){
			$status=new CSpan(S_EXECUTED,"green");
		}
		else if($row["status"] == ALERT_STATUS_NOT_SENT){
			$status=new CSpan(S_IN_PROGRESS,"orange");
		}
		else{
			$status=new CSpan(S_NOT_SENT,"red");
		}

		$message = array(bold(S_COMMAND.':'));
		$msg = explode("\n",$row['message']);
		foreach($msg as $m){
			array_push($message, BR(), $m);
		}

		if(empty($row["error"])){
			$error=new CSpan(SPACE,"off");
		}
		else{
			$error=new CSpan($row["error"],"on");
		}
		
		$table->AddRow(array(
			get_node_name_by_elid($row['alertid']),
			new CCol($time, 'top'),
			new CCol($status, 'top'),
			new CCol($message, 'wraptext top'),
			new CCol($error, 'wraptext top')));
	}

return $table;
}

// Author: Aly
function get_actions_hint_by_eventid($eventid,$status=NULL){
	$available_triggers = get_accessible_triggers(PERM_READ_ONLY, PERM_RES_IDS_ARRAY);
	
	$tab_hint = new CTableInfo(S_NO_ACTIONS_FOUND);
	$tab_hint->AddOption('style', 'width: 300px;');
	$tab_hint->SetHeader(array(
			is_show_subnodes() ? S_NODES : null,
			S_USER,
			S_DETAILS,
			S_STATUS
			));
/*			
	$sql = 'SELECT DISTINCT a.alertid,mt.description,a.sendto,a.status,u.alias,a.retries '.
			' FROM events e,users u,alerts a'.
			' left join media_type mt on mt.mediatypeid=a.mediatypeid'.
			' WHERE a.eventid='.$eventid.
				(is_null($status)?'':' AND a.status='.$status).
				' AND e.eventid = a.eventid'.
				' AND a.alerttype IN ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
				' AND '.DBcondition('e.objectid',$available_triggers).
				' AND '.DBin_node('a.alertid').
				' AND u.userid=a.userid '.
			' ORDER BY mt.description';
//*/
	$sql = 'SELECT DISTINCT a.alertid,mt.description,u.alias,a.subject,a.message,a.sendto,a.status,a.retries,a.alerttype '.
			' FROM events e,alerts a '.
				' LEFT JOIN users u ON u.userid=a.userid '.
				' LEFT JOIN media_type mt ON mt.mediatypeid=a.mediatypeid'.
			' WHERE a.eventid='.$eventid.
				(is_null($status)?'':' AND a.status='.$status).
				' AND e.eventid = a.eventid'.
				' AND a.alerttype IN ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
				' AND '.DBcondition('e.objectid',$available_triggers).
				' AND '.DBin_node('a.alertid').
			' ORDER BY a.alertid';
	$result=DBselect($sql);

	while($row=DBfetch($result)){

		if($row["status"] == ALERT_STATUS_SENT){
			$status=new CSpan(S_SENT,"green");
			$retries=new CSpan(SPACE,"green");
		}
		else if($row["status"] == ALERT_STATUS_NOT_SENT){
			$status=new CSpan(S_IN_PROGRESS,"orange");
			$retries=new CSpan(ALERT_MAX_RETRIES - $row["retries"],"orange");
		}
		else{
			$status=new CSpan(S_NOT_SENT,"red");
			$retries=new CSpan(0,"red");
		}
		
		switch($row['alerttype']){
			case ALERT_TYPE_MESSAGE:
				$message = empty($row['description'])?'-':$row['description'];
				break;
			case ALERT_TYPE_COMMAND:
				$message = array(bold(S_COMMAND.':'));
				$msg = explode("\n",$row['message']);
				foreach($msg as $m){
					array_push($message, BR(), $m);
				}
				break;
			default:
				$message = '-';
		}
		
		$tab_hint->AddRow(array(
			get_node_name_by_elid($row['alertid']),
			empty($row['alias'])?' - ':$row['alias'],
			$message,
			$status
		));
	}

return $tab_hint;
}

function get_event_actions_status($eventid){
// Actions								
	$actions= new CTable(' - ');

	$sql='SELECT COUNT(a.alertid) as cnt_all'.
			' FROM alerts a '.
			' WHERE a.eventid='.$eventid.
				' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')';
			
	$alerts=DBfetch(DBselect($sql));

	if(isset($alerts['cnt_all']) && ($alerts['cnt_all'] > 0)){
		$mixed = 0;
// Sent
		$sql='SELECT COUNT(a.alertid) as sent '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_SENT;

		$tmp=DBfetch(DBselect($sql));
		$alerts['sent'] = $tmp['sent'];
		$mixed+=($alerts['sent'])?ALERT_STATUS_SENT:0;
// In progress
		$sql='SELECT COUNT(a.alertid) as inprogress '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_NOT_SENT;

		$tmp=DBfetch(DBselect($sql));
		$alerts['inprogress'] = $tmp['inprogress'];
// Failed
		$sql='SELECT COUNT(a.alertid) as failed '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_FAILED;

		$tmp=DBfetch(DBselect($sql));
		$alerts['failed'] = $tmp['failed'];
		$mixed+=($alerts['failed'])?ALERT_STATUS_FAILED:0;

		if($alerts['inprogress']){
			$status = new CSpan(S_IN_PROGRESS,'orange');
		}
		else if(ALERT_STATUS_SENT == $mixed){
			$status = new CSpan(S_OK,'green');
		}
		else if(ALERT_STATUS_FAILED == $mixed){
			$status = new CSpan(S_FAILED,'red');
		}
		else{
			$tdl = new CCol(($alerts['sent'])?(new CSpan($alerts['sent'],'green')):SPACE);
			$tdl->AddOption('width','10');
			
			$tdr = new CCol(($alerts['failed'])?(new CSpan($alerts['failed'],'red')):SPACE);
			$tdr->AddOption('width','10');

			$status = new CRow(array($tdl,$tdr));
		}

		$actions->AddRow($status);
	}
	
return $actions;
}

function get_event_actions_stat_hints($eventid){
	$actions= new CTable(' - ');

	$sql='SELECT COUNT(a.alertid) as cnt '.
			' FROM alerts a '.
			' WHERE a.eventid='.$eventid.
				' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')';

			
	$alerts=DBfetch(DBselect($sql));

	if(isset($alerts['cnt']) && ($alerts['cnt'] > 0)){
		$sql='SELECT COUNT(a.alertid) as sent '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_SENT;

		$alerts=DBfetch(DBselect($sql));

		$alert_cnt = new CSpan($alerts['sent'],'green');
		if($alerts['sent']){
			$hint=get_actions_hint_by_eventid($eventid,ALERT_STATUS_SENT);
			$alert_cnt->SetHint($hint);
		}
		$tdl = new CCol(($alerts['sent'])?$alert_cnt:SPACE);
		$tdl->AddOption('width','10');

		$sql='SELECT COUNT(a.alertid) as inprogress '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_NOT_SENT;

		$alerts=DBfetch(DBselect($sql));

		$alert_cnt = new CSpan($alerts['inprogress'],'orange');
		if($alerts['inprogress']){
			$hint=get_actions_hint_by_eventid($eventid,ALERT_STATUS_NOT_SENT);
			$alert_cnt->SetHint($hint);
		}
		$tdc = new CCol(($alerts['inprogress'])?$alert_cnt:SPACE);
		$tdc->AddOption('width','10');

		$sql='SELECT COUNT(a.alertid) as failed '.
				' FROM alerts a '.
				' WHERE a.eventid='.$eventid.
					' AND a.alerttype in ('.ALERT_TYPE_MESSAGE.','.ALERT_TYPE_COMMAND.')'.
					' AND a.status='.ALERT_STATUS_FAILED;

		$alerts=DBfetch(DBselect($sql));

		$alert_cnt = new CSpan($alerts['failed'],'red');
		if($alerts['failed']){
			$hint=get_actions_hint_by_eventid($eventid,ALERT_STATUS_FAILED);
			$alert_cnt->SetHint($hint);
		}

		$tdr = new CCol(($alerts['failed'])?$alert_cnt:SPACE);
		$tdr->AddOption('width','10');
		
		$actions->AddRow(array($tdl,$tdc,$tdr));
	}
return $actions;
}
?>
