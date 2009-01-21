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
	class CFormTable extends CForm{
/* private *//*
		var $align;
		var $title;
		var $help;*/
/* protected *//*
		var $top_items = array();
		var $center_items = array();
		var $bottom_items = array();*/
/* public */
		function CFormTable($title=null, $action=null, $method=null, $enctype=null, $form_variable=null){
			global  $_REQUEST;

			$this->top_items = array();
			$this->center_items = array();
			$this->bottom_items = array();
			$this->tableclass = 'formtable';

			if( null == $method ){
				$method = 'post';
			}

			if( null == $form_variable ){
				$form_variable = 'form';
			}

			parent::CForm($action,$method,$enctype);
			$this->setTitle($title);
			$this->setAlign('center');
			$this->setHelp();

//			$frm_link = new CLink();
//			$frm_link->setName("formtable");
//			$this->addItemToTopRow($frm_link);
			
			$this->addVar($form_variable, get_request($form_variable, 1));
			$this->addVar('form_refresh',get_request('form_refresh',0)+1);

			$this->bottom_items = new CCol(SPACE,'form_row_last');
		        $this->bottom_items->setColSpan(2);
		}
		
		function setAction($value){
			
			if(is_string($value))
				return parent::setAction($value);
			elseif(is_null($value))
				return parent::setAction($value);
			else
				return $this->error("Incorrect value for SetAction [$value]");
		}
		
		function setName($value){
			if(!is_string($value)){
				return $this->error("Incorrect value for SetAlign [$value]");
			}
			$this->addOption('name',$value);
			$this->addOption('id',$value);
		return true;
		}
		
		function setAlign($value){
			if(!is_string($value)){
				return $this->error("Incorrect value for SetAlign [$value]");
			}
			return $this->align = $value;
		}

		function setTitle($value=NULL){
			if(is_null($value)){
				unset($this->title);
				return 0;
			}
/*
			elseif(!is_string($value)){
				return $this->error("Incorrect value for SetTitle [$value]");
			}
			$this->title = nbsp($value);
*/
			$this->title = unpack_object($value);
		}
		
		function setHelp($value=NULL){
			if(is_null($value)) {
				$this->help = new CHelp();
			} 
			else if(strtolower(get_class($value)) == 'chelp') {
				$this->help = $value;
			} 
			else if(is_string($value)) {
				$this->help = new CHelp($value);
				if($this->getName()==NULL)
					$this->setName($value);
			} 
			else {
				return $this->error("Incorrect value for SetHelp [$value]");
			}
			return 0;
		}
		
		function addVar($name, $value){
			$this->addItemToTopRow(new CVar($name, $value));
		}
		
		function addItemToTopRow($value){
			array_push($this->top_items, $value);
		}
		
		function addRow($item1, $item2=NULL, $class=NULL){
			if(strtolower(get_class($item1)) == 'crow'){
			
			} 
			else if(strtolower(get_class($item1)) == 'ctable'){
				$td = new CCol($item1,'form_row_c');
				$td->setColSpan(2);
				
				$item1 = new CRow($td);
			} 
			else{
				$tmp = $item1;
				if(is_string($item1)){
					$item1=nbsp($item1);
				}
				
				if(empty($item1)) $item1 = SPACE;
				if(empty($item2)) $item2 = SPACE;
				
				$item1 = new CRow(
								array(
									new CCol($item1,'form_row_l'),
									new CCol($item2,'form_row_r')
								),
								$class);
			}

			array_push($this->center_items, $item1);
		}
		
		function addSpanRow($value, $class=NULL){
			if(is_string($value))
				$item1=nbsp($value);

			if(is_null($value)) $value = SPACE;
			if(is_null($class)) $class = 'form_row_c';

			$col = new CCol($value,$class);
		        $col->setColSpan(2);
			array_push($this->center_items,new CRow($col));
		}
		
		
		function addItemToBottomRow($value){
			$this->bottom_items->addItem($value);
		}

		function setTableClass($class){
			if(is_string($class)){
				$this->tableclass = $class;
			}
		}
		
/* protected */
		function BodyToString(){
			parent::BodyToString();

			$tbl = new CTable(NULL,$this->tableclass);

			$tbl->setOddRowClass('form_odd_row');
			$tbl->setEvenRowClass('form_even_row');
			$tbl->setCellSpacing(0);
			$tbl->setCellPadding(1);
			$tbl->setAlign($this->align);
# add first row
			$col = new CCol(NULL,'form_row_first');
			$col->setColSpan(2);
			
			if(isset($this->help))			$col->addItem($this->help);
			if(isset($this->title))		 	$col->addItem($this->title);
			foreach($this->top_items as $item)	$col->addItem($item);
			
			$tbl->setHeader($col);
# add last row
			$tbl->setFooter($this->bottom_items);
# add center rows
			foreach($this->center_items as $item){
				$tbl->addRow($item);
			}
		return $tbl->ToString();
		}
	}
?>
