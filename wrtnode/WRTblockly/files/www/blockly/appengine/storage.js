/**
 * @license
 * Visual Blocks Editor
 *
 * Copyright 2012 Google Inc.
 * https://blockly.googlecode.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview Loading and saving blocks with localStorage and cloud storage.
 * @author q.neutron@gmail.com (Quynh Neutron)
 */
'use strict';

// Create a namespace.
var BlocklyStorage = {};
var Sys = {};
var ua = navigator.userAgent.toLowerCase();
var s;
/**
 * Backup code blocks to localStorage.
 * @private
 */
BlocklyStorage.backupBlocks_ = function() {
  if ('localStorage' in window) {
    var xml = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
    // Gets the current URL, not including the hash.
    var url = window.location.href.split('#')[0];
    window.localStorage.setItem(url, Blockly.Xml.domToText(xml));
  }
};

/**
 * Bind the localStorage backup function to the unload event.
 */
BlocklyStorage.backupOnUnload = function() {
  window.addEventListener('unload', BlocklyStorage.backupBlocks_, false);
};

/**
 * Restore code blocks from localStorage.
 */
BlocklyStorage.restoreBlocks = function() {
  var url = window.location.href.split('#')[0];
  if ('localStorage' in window && window.localStorage[url]) {
    var xml = Blockly.Xml.textToDom(window.localStorage[url]);
    Blockly.Xml.domToWorkspace(Blockly.getMainWorkspace(), xml);
  }
};

/**
 * Save blocks to database and return a link containing key to XML.
 */
BlocklyStorage.link = function() {
  var xml = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
  var data = Blockly.Xml.domToText(xml);
  BlocklyStorage.makeRequest_('/storage', 'xml', data);
};

/**
 * Retrieve XML text from database using given key.
 * @param {string} key Key to XML, obtained from href.
 */
BlocklyStorage.retrieveXml = function(key) {
  var xml = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
  BlocklyStorage.makeRequest_('/storage', 'key', key);
};

/**
 * Global reference to current AJAX request.
 * @type XMLHttpRequest
 * @private
 */
BlocklyStorage.httpRequest_ = null;

/**
 * Fire a new AJAX request.
 * @param {string} url URL to fetch.
 * @param {string} name Name of parameter.
 * @param {string} content Content of parameter.
 * @private
 */
BlocklyStorage.makeRequest_ = function(url, name, content) {
  if (BlocklyStorage.httpRequest_) {
    // AJAX call is in-flight.
    BlocklyStorage.httpRequest_.abort();
  }
  BlocklyStorage.httpRequest_ = new XMLHttpRequest();
  BlocklyStorage.httpRequest_.name = name;
  BlocklyStorage.httpRequest_.onreadystatechange =
      BlocklyStorage.handleRequest_;
  BlocklyStorage.httpRequest_.open('POST', url);
  BlocklyStorage.httpRequest_.setRequestHeader('Content-Type',
      'application/x-www-form-urlencoded');
  BlocklyStorage.httpRequest_.send(name + '=' + encodeURIComponent(content));
};

/**
 * Callback function for AJAX call.
 * @private
 */
BlocklyStorage.handleRequest_ = function() {
  if (BlocklyStorage.httpRequest_.readyState == 4) {
    if (BlocklyStorage.httpRequest_.status != 200) {
      BlocklyStorage.alert(BlocklyStorage.HTTPREQUEST_ERROR + '\n' +
          'httpRequest_.status: ' + BlocklyStorage.httpRequest_.status);
    } else {
      var data = BlocklyStorage.httpRequest_.responseText.trim();
      if (BlocklyStorage.httpRequest_.name == 'xml') {
        window.location.hash = data;
        BlocklyStorage.alert(BlocklyStorage.LINK_ALERT.replace('%1',
            window.location.href));
      } else if (BlocklyStorage.httpRequest_.name == 'key') {
        if (!data.length) {
          BlocklyStorage.alert(BlocklyStorage.HASH_ERROR.replace('%1',
              window.location.hash));
        } else {
          BlocklyStorage.loadXml_(data);
        }
      }
      BlocklyStorage.monitorChanges_();
    }
    BlocklyStorage.httpRequest_ = null;
  }
};

/**
 * Start monitoring the workspace.  If a change is made that changes the XML,
 * clear the key from the URL.  Stop monitoring the workspace once such a
 * change is detected.
 * @private
 */
BlocklyStorage.monitorChanges_ = function() {
  var startXmlDom = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
  var startXmlText = Blockly.Xml.domToText(startXmlDom);
  function change() {
    var xmlDom = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
    var xmlText = Blockly.Xml.domToText(xmlDom);
    if (startXmlText != xmlText) {
      window.location.hash = '';
      Blockly.removeChangeListener(bindData);
    }
  }
  var bindData = Blockly.addChangeListener(change);
};

/**
 * Load blocks from XML.
 * @param {string} xml Text representation of XML.
 * @private
 */
BlocklyStorage.loadXml_ = function(xml) {
  try {
    xml = Blockly.Xml.textToDom(xml);
  } catch (e) {
    BlocklyStorage.alert(BlocklyStorage.XML_ERROR + '\nXML: ' + xml);
    return;
  }
  // Clear the workspace to avoid merge.
  Blockly.getMainWorkspace().clear();
  Blockly.Xml.domToWorkspace(Blockly.getMainWorkspace(), xml);
};

/**
 * Present a text message to the user.
 * Designed to be overridden if an app has custom dialogs, or a butter bar.
 * @param {string} message Text to alert.
 */
BlocklyStorage.alert = function(message) {
  window.alert(message);
};

function loadFile(x)
{
	discard();
	  $.ajax({
	  type: "GET",
	url: "/cgi-bin/blockly_python/load?loadName="+x,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  if (json.result == "error") {
			alert("调用出错");
		  }else
		  {
			  var text=decodeURIComponent(json.xml);
			  //alert(text);
			  var xml = Blockly.Xml.textToDom(text);
			  Blockly.Xml.domToWorkspace(Blockly.getMainWorkspace(), xml);
		  }
	  },
	error: function(error) {
	alert("调出错" + error.responseText);
	  }
	});
	return false;
}
  function deleteFile(x)
{
	  $.ajax({
	  type: "GET",
	url: "/cgi-bin/blockly_python/delete?deleteName="+x,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  if (json.result == "error") {
			alert("调用出错");
		  }
		  if (json.result == "success") {
			
		  }
	  },
	error: function(error) {
	alert("调用出错" + error.responseText);
	  }
	});
	load_file_name();
	return false;
}

function deleteFile(x,obj)
{
	  $.ajax({
	  type: "GET",
	url: "/cgi-bin/blockly_python/delete?deleteName="+x,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  if (json.result == "error") {
			alert("调用出错");
		  }
		  if (json.result == "success") {
			
		  }
	  },
	error: function(error) {
	alert("调用出错" + error.responseText);
	  }
	});
	var tr=obj.parentNode.parentNode; 
	var tbody=tr.parentNode; 
	tbody.removeChild(tr);
	return false;
}

  function uploadXML()
  {
	  var xml = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
	  var xml_text=Blockly.Xml.domToText(xml);
	  var text={};
	  text["text"]=xml_text;
	  var fname=prompt("请输入程序名称：");
	  while(fname=="")
	  {
		  alert("文件名不能为空！");
		  fname=prompt("请输入程序名称：");
	  }
	  var description=prompt("文件简单描述：");
	  	  if(description=="")
	  {
		  alert("描述不能为空！");
		  description=prompt("文件简单描述：");
	  }
	  text["name"]=escape(fname)+"_"+escape(description)+".xml";
	  $.post("/cgi-bin/blockly_python/uploadXML",text , function(json){
	 if(json.result=="error")
	 {
		 alert("保存失败");
	 }
	 else
	 {
		 alert("保存成功");
	  }
	 },
	 "json");
	 var tbl = document.getElementById("tab_xml"); 
	var obj = tbl.insertRow(tbl.rows.length);
	var file_name = fname+'_'+description+'.xml';
	//obj.insertCell().innerHTML ="load";
	//obj.insertCell().innerHTML ="delete";
	var bt_load="<input type=\"button\" onClick = \"loadFile(\'"+escape(file_name)+"\')\" value=\"加载\">";
	
	var bt_delete="<input type=\"button\" onClick = \"deleteFile(\'"+escape(file_name)+"\',this)\" value=\"删除\">";
	if(Sys.chrome||Sys.ie)
	{
		obj.insertCell().innerHTML = fname;
		obj.insertCell().innerHTML = description;
		obj.insertCell().innerHTML =bt_load ;	
		obj.insertCell().innerHTML =bt_delete;	
	}
	else 
	{
		obj.insertCell().innerHTML =bt_delete;
		obj.insertCell().innerHTML =bt_load ;
		obj.insertCell().innerHTML = description;
		obj.insertCell().innerHTML = fname;		
	}
  }
  function deletetablerow()
  {
	 var tbl = document.getElementById("tab_xml"); 
     while(tbl.rows.length>1)
	{
		tbl.removeChild(tbl.firstChild);
    }
	return false;
  }
  function saveXML()
  {
		if ('localStorage' in window) {
		  var xml = Blockly.Xml.workspaceToDom(Blockly.getMainWorkspace());
		  // Gets the current URL, not including the hash.
		  var url = window.location.href.split('#')[0];
		  window.localStorage.setItem(url, Blockly.Xml.domToText(xml));
		}  
  }
  function loadXML()
  {
	  var url = window.location.href.split('#')[0];
	  if ('localStorage' in window && window.localStorage[url]) {
		var xml = Blockly.Xml.textToDom(window.localStorage[url]);
		Blockly.Xml.domToWorkspace(Blockly.getMainWorkspace(), xml);
	  }
  }
/*function init() {
//window.onbeforeunload = function() {
//  return 'Leaving this page will result in the loss of your work.';
//};

var loadInput = document.getElementById('load');
loadInput.addEventListener('change', load, false);
document.getElementById('fakeload').onclick = function() {
  loadInput.click();
};
//load from url parameter (single param)
//http://stackoverflow.com/questions/2090551/parse-query-string-in-javascript
}
*/
function load_file_name() 
{
	$.ajax({
	type: "GET", 
    url: "/cgi-bin/blockly_python/xmlName",
    dataType: "json",
    contentType: "application/json; charset=utf-8",
    success: function(json) {
	  if(json=="{}")
	  {
		  document.getElementById("div_xml").innerHTML="<p>还没有保存内容！</p>"
	  }
	  else
	  {
		  var tbl = document.getElementById("tab_xml");
		  while(tbl.rows.length>1)
		  {
			 tbl.removeChild(tbl.firstChild);
    	  }
		  
         (s = ua.match(/msie ([\d.]+)/)) ? Sys.ie = s[1] :
         (s = ua.match(/firefox\/([\d.]+)/)) ? Sys.firefox = s[1] :
         (s = ua.match(/chrome\/([\d.]+)/)) ? Sys.chrome = s[1] :
         (s = ua.match(/opera.([\d.]+)/)) ? Sys.opera = s[1] :
         (s = ua.match(/version\/([\d.]+).*safari/)) ? Sys.safari = s[1] : 0;
		 if (Sys.chrome||Sys.ie)
		 {
			for(var key in json)
			{
				var obj = tbl.insertRow(tbl.rows.length);
				var file_name = json[key];
				var name=file_name.split('_');//name[0]为名称，name[1]为描述
  
				//obj.insertCell().innerHTML ="load";
				//obj.insertCell().innerHTML ="delete";
				var bt_load="<input type=\"button\" onClick = \"loadFile(\'"+file_name+"\',this)\" value=\"加载\">";
			   
				var bt_delete="<input type=\"button\" onClick = \"deleteFile(\'"+file_name+"\',this)\" value=\"删除\">";
				
				
				  
				obj.insertCell().innerHTML = unescape(name[0]);
				obj.insertCell().innerHTML = unescape(name[1].substring(0,name[1].length-4));
				
				  obj.insertCell().innerHTML =bt_load ;
				obj.insertCell().innerHTML =bt_delete;
			}
		 }
		 else
		 {
			for(var key in json)
			{
				var obj = tbl.insertRow(tbl.rows.length);
				var file_name = json[key];
				var name=file_name.split('_');//name[0]为名称，name[1]为描述
  
				//obj.insertCell().innerHTML ="load";
				//obj.insertCell().innerHTML ="delete";
				var bt_load="<input type=\"button\" onClick = \"loadFile(\'"+file_name+"\',this)\" value=\"加载\">";
			   
				var bt_delete="<input type=\"button\" onClick = \"deleteFile(\'"+file_name+"\',this)\" value=\"删除\">";
				
				
				  obj.insertCell().innerHTML =bt_delete;
				  obj.insertCell().innerHTML =bt_load ;
				
				obj.insertCell().innerHTML = unescape(name[1].substring(0,name[1].length-4));
				obj.insertCell().innerHTML = unescape(name[0]);
			
			}
		 }
	  }
	},
    error: function(error) {
	  alert("调用出错");

	}
  });	
}

function discard() {
  var count = Blockly.mainWorkspace.getAllBlocks().length;
  if (count < 2 || window.confirm('Delete all ' + count + ' blocks?')) {
    Blockly.mainWorkspace.clear();
  }
}