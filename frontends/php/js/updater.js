// JavaScript Document
/*
** ZABBIX
** Copyright (C) 2000-2008 SIA Zabbix
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
// DOM obj update class
// Author: Aly
var updater = {
objlist:		new Array(),			// list of objects
optlist :		new Array(),			// object params, list
interval:		10,						// update interval in sec
inloadobj:		new Array(),			// array containing updated obj and statuses

	setObj4Update: function(id,frequency,url,params){
		var obj = document.getElementById(id);
		if(typeof(obj) == 'undefined') return false; 
	
		var obj4update = {
			'id': 		id,
			'url': 		url,
			'params': 	params,
			'interval': frequency,
			'lastupdate': 0,
			'ready': true
		}
		
		if(typeof(this.optlist[id]) == 'undefined'){
			this.objlist.push(id);
		}
		this.optlist[id] = obj4update;
	},
	
	check4Update: function(){
		if(this.objlist.length > 0){
			var dt = new Date();
			var now = parseInt(dt.getTime()/1000);
			
			for(var i=0; i < this.objlist.length; i++){
				if((typeof(this.optlist[this.objlist[i]]) != 'undefined') && !empty(this.optlist[this.objlist[i]])){
	//				alert(Math.abs(now - this.optlist[this.objlist[i]].lastupdate));
					if(this.optlist[this.objlist[i]].ready && (this.optlist[this.objlist[i]].interval <= Math.abs(now - this.optlist[this.objlist[i]].lastupdate))){
						this.update(this.optlist[this.objlist[i]],now);
					}
				}
			}
		}
		setTimeout('updater.check4Update();',(this.interval*1000));
	},
	
	update: function(obj4update,time){
		obj4update.ready = false;

		var uri = new url(obj4update.url);
		new Ajax.Updater(obj4update.id, uri.getUrl(),//obj4update.url,
			{
				method: 'post',
				'parameters':	obj4update.params,
				'evalScripts': true,
				'onSuccess': function(resp){ 
						var headers = resp.getAllResponseHeaders(); 
//						alert(headers);
						if(headers.indexOf('Ajax-response: false') > -1){
							resp.responseText = $(obj4update.id).innerHTML;
//							return false;
						}
						
						obj4update.lastupdate = time; 
						obj4update.ready = true;
					},	//SDI(resp.responseText);
				'onFailure': function(){ document.location = uri.getPath()+'?'+Object.toQueryString(obj4update.params); }
			});	
	},

	onetime_update: function(obj,update_url){
		obj = $(obj);
		
		if(!is_string(update_url)){
			var upd_form = $(update_url);
			if(('undefined' != upd_form.tagName) && (upd_form.tagName.toLowerCase() == 'form')){
				update_url = '?'+upd_form.serialize();
			}
		}
				
		this.setLoadingImg(obj);
		
		var uri = new url(update_url);
		var params = {
			'favobj': 	'refresh',
			'favid': 	obj.id,
			'output':	'html'
		}
		
		if('undefined' != this.optlist[obj.id]){
			var dt = new Date();
			this.optlist[obj.id].lastupdate = parseInt(dt.getTime()/1000)
			this.optlist[obj.id].url = update_url+(empty(update_url)?'?':'&')+Object.toQueryString(params);
		}

		var ajax_result = new Ajax.Updater(obj, update_url,
			{
				method: 		'post',
				'parameters':	params,
				'evalScripts': 	true,
				'onSuccess': 	function(resp){ 
						var headers = resp.getAllResponseHeaders();  //	alert(headers);

						if(headers.indexOf('Ajax-response: false') > -1){
							resp.responseText = $(obj).innerHTML;
						}
						updater.rmwLoadingImg(obj);
					},	//	alert(resp.responseText);
				'onFailure': 	function(transport){ 
						document.location = uri.getUrl(); 
					}
			});

	return !ajax_result.success;
	},
	
	setLoadingImg: function(obj){
		obj = $(obj);
		if('undefined' == typeof(this.inloadobj[obj.id])){
			this.inloadobj[obj.id] = new Array();
			this.inloadobj[obj.id].status = false;
			this.inloadobj[obj.id].divref = null;
		}
		
		if(this.inloadobj[obj.id].status == false){
			this.inloadobj[obj.id].status = true;
			
			var ddiv = document.createElement('div');		
			document.body.appendChild(ddiv);
			
			ddiv.className = 'onajaxload';
		}
		else if(this.inloadobj[obj.id].status == true){
			var ddiv = this.inloadobj[obj.id].divref;
		}
		else{
			return true;
		}
		
		var obj_params = getPosition(obj);
		obj_params.height = obj.offsetHeight;
		obj_params.width = obj.offsetWidth;
		
		Element.extend(ddiv);
		ddiv.setStyle({ 'top': obj_params.top+'px', 
					  	'left': obj_params.left+'px',
						'width': obj_params.width+'px',
						'height': obj_params.height+'px'
						});
		
		this.inloadobj[obj.id].divref = ddiv;
	},

	rmwLoadingImg: function(obj){
		if('undefined' != typeof(this.inloadobj[obj.id])){
			this.inloadobj[obj.id].status = false;
			this.inloadobj[obj.id].divref.style.cursor = 'auto';
			
			document.body.removeChild(this.inloadobj[obj.id].divref);
			this.inloadobj[obj.id].divref = null;
		}
	}
}
