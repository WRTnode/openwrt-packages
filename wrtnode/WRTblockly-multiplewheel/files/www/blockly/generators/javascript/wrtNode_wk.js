/**
 * @the block for wrtNOde
 * @author wankge90@qq.com(wangke)
 */
'use strict';
//function used  


Blockly.getreturnnum=function(type)
{
	var temp = 0;
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/getreturn?type="+type,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  temp=json.temp;
	  },
	error: function(error) {
		alert("调用出错"+"/cgi-bin/blockly_python/getreturn?type="+type);
		temp=-255;
	  }
	});
	return temp;

}

Blockly.getnoreturnnum=function(type)
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/getnoreturn?type="+type,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  temp=json.temp;
	  },
	error: function(error) {
		alert("调用出错"+"/cgi-bin/blockly_python/getnoreturn?type="+type);
	  }
	});
}
Blockly.move=function(type1,num)
{
	if(type1==0)
	{
		num=-num;
	}
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/car_python/g?value="+num,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		
	  },
	error: function(error) {
	  }
	});
	var finish1=true;
	while(finish1)
	{
		$.ajax({
		type: "GET",
		async:false,
		url: "/cgi-bin/car_python/f",
		dataType: "json",
		contentType: "application/json; charset=utf-8",
		success: function(json) {
			if(json.ans=='0')
			{
				finish1=false;
			}

		  },
		error: function(error) {
			alert("error");
		  }
		});		
	}
	
}
Blockly.turn=function(type,num)
{
	if(type==0)
	{
		num=-num;
	}
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/car_python/t?value="+num,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
	var finish1=true;
	while(finish1)
	{
		$.ajax({
		type: "GET",
		async:false,
		url: "/cgi-bin/car_python/f",
		dataType: "json",
		contentType: "application/json; charset=utf-8",
		success: function(json) {
			if(json.ans=='0')
			{
				finish1=false;
			}

		  },
		error: function(error) {
			alert("error");
		  }
		});		
	}
	
}



Blockly.setdunio=function(type,io,num1,num2)
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/setdunio?type="+type+"&io="+io+"&num1="+num1+"&num2="+num2,
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
		alert("调用出错"+"/cgi-bin/blockly_python/setdunio?type="+type+"&io="+io+"&num1="+num1+"&num2="+num2);
	  }
	});
}
Blockly.readdunio=function(type,io)
{
	var temp="3400"
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/setdunio?type="+type+"&io="+io+"&num1=0&num2=0",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
		  temp=json.ans;
	  },
	error: function(error) {
		alert("调用出错"+"/cgi-bin/blockly_python/setdunio?type="+type+"&io="+io+"&num1=0&num2=0");
	  }
	});
	return temp;
}
Blockly.initduno=function()
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/init",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
}
Blockly.set13pwm=function()
{
	var code = '';
    $.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/set13pwm",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
}
Blockly.delay=function(time)
{
	var start=new Date().getTime();
    while(true)
	{
		 if(new Date().getTime()-start>time)
		 {
		 	 break;
		 }
	}
}
Blockly.set13output=function()
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/set13out",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
}
Blockly.set13outputhigh=function()
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/set13outlow",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
}
Blockly.set13outputlow=function()
{
	$.ajax({
	type: "GET",
	async:false,
	url: "/cgi-bin/blockly_python/set13outhigh",
	dataType: "json",
	contentType: "application/json; charset=utf-8",
	success: function(json) {
	  },
	error: function(error) {
	  }
	});
}

Blockly.JavaScript['wrtnode_temp'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code="Blockly.getreturnnum('temp')";

  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrtnode_humi'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code="Blockly.getreturnnum('humi')";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrtnode_pres'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code="Blockly.getreturnnum('pres')";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrtnode_dist'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code="Blockly.getreturnnum('dist')";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrtnode_init'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code="Blockly.initduno();\n";
  return code;
};

/*Blockly.JavaScript['wrt_high'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '...';
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrt_low'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '...';
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};*/

Blockly.JavaScript['wrt_output1'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = 13;
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrt_13output'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = 'Blockly.set13output();\n';
  return code;
};

Blockly.JavaScript['wrt_13outputhigh'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = 'Blockly.set13outputhigh();\n';
  return code;
};

Blockly.JavaScript['wrt_13outputlow'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = 'Blockly.set13outputlow();\n';
  return code;
};

Blockly.JavaScript['wrt_13outpwm'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = 'Blockly.set13pwm();\n';

  return code;
};

Blockly.JavaScript['base_delay'] = function(block) {
  var value_time = Blockly.JavaScript.valueToCode(block, 'time', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.delay("+value_time+");\n";
  return code;
};
Blockly.JavaScript['input_highlow'] = function(block) {
  var dropdown_input = block.getFieldValue('input');
  // TODO: Assemble JavaScript into code variable.
  // TODO: Change ORDER_NONE to the correct strength.
  return [dropdown_input, Blockly.JavaScript.ORDER_NONE];
};
Blockly.JavaScript['inout_digital_write'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var dropdown_input = block.getFieldValue('input');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('digital_write',"+dropdown_io+","+dropdown_input+",0);\n";
  return code;
};
Blockly.JavaScript['inout_digital_read'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.readdunio('digital_read',"+dropdown_io+")";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['inout_analog_read'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.readdunio('analog_read',"+dropdown_io+")";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['inout_analog_write'] = function(block) {
  var value_v = Blockly.JavaScript.valueToCode(block, 'v', Blockly.JavaScript.ORDER_ATOMIC);
  var dropdown_io = block.getFieldValue('io');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('analog_write',"+dropdown_io+","+value_v+",0);\n";
  return code;
};

Blockly.JavaScript['inout_dialog_freq'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var value_v = Blockly.JavaScript.valueToCode(block, 'v', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('dialog_freq',"+dropdown_io+","+value_v+",0);\n";
  return code;
};

Blockly.JavaScript['inout_dialog_cover'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var value_v = Blockly.JavaScript.valueToCode(block, 'v', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('cover',"+dropdown_io+","+value_v+",0)";
  return code;
};

Blockly.JavaScript['inout_dialog_freqcover'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var value_fre = Blockly.JavaScript.valueToCode(block, 'fre', Blockly.JavaScript.ORDER_ATOMIC);
  var value_cov = Blockly.JavaScript.valueToCode(block, 'cov', Blockly.JavaScript.ORDER_ATOMIC);
  var code = "Blockly.setdunio('freqcover',"+dropdown_io+","+value_fre+","+value_cov+");\n";
  // TODO: Assemble JavaScript into code variable.
  return code;
};

Blockly.JavaScript['wrt_dunio_iosetgpio'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var dropdown_type = block.getFieldValue('type');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('iosetgpio',"+dropdown_io+","+dropdown_type+",0);\n";
  return code;
};





Blockly.JavaScript['wrt_dunio_ioset'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  var dropdown_type = block.getFieldValue('type');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('ioset',"+dropdown_io+","+dropdown_type+",0);\n";
  return code;
};

Blockly.JavaScript['wrt_dunio_iosetmn'] = function(block) {
  var dropdown_io = block.getFieldValue('io');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('iosetadc',"+dropdown_io+",0,0);\n";
  return code;
};

Blockly.JavaScript['wrt_dunio_setpwm'] = function(block) {
  var value_io = Blockly.JavaScript.valueToCode(block, 'io', Blockly.JavaScript.ORDER_ATOMIC);
  var value_rate = Blockly.JavaScript.valueToCode(block, 'rate', Blockly.JavaScript.ORDER_ATOMIC);
  var value_cover = Blockly.JavaScript.valueToCode(block, 'cover', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('freqcover',"+dropdown_io+","+value_rate+","+value_cover+");\n";
  return code;
};

Blockly.JavaScript['wrt_dunio_setiogene'] = function(block) {
  var value_name = Blockly.JavaScript.valueToCode(block, 'NAME', Blockly.JavaScript.ORDER_ATOMIC);
  var dropdown_type = block.getFieldValue('type');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('iosetgpio',"+value_name+","+dropdown_type+",0);\n";
  return code;
};


Blockly.JavaScript['wrt_dunio_setioout'] = function(block) {
  var value_io = Blockly.JavaScript.valueToCode(block, 'io', Blockly.JavaScript.ORDER_ATOMIC);
  var value_num = Blockly.JavaScript.valueToCode(block, 'num', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.setdunio('digital_write',"+value_io+","+value_num+",0);\n";
  return code;
};

Blockly.JavaScript['wrt_dunio_setioreada'] = function(block) {
  var value_io = Blockly.JavaScript.valueToCode(block, 'io', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
    var code = "Blockly.setdunio('iosetadc',"+value_io+",0,0);\n";
  // TODO: Change ORDER_NONE to the correct strength.
  return code;
};

Blockly.JavaScript['wrt_dunio_readiod'] = function(block) {
  var value_io = Blockly.JavaScript.valueToCode(block, 'io', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.readdunio('digital_read',"+value_io+")";
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['wrt_dunio_readioa'] = function(block) {
  var value_io = Blockly.JavaScript.valueToCode(block, 'io', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.readdunio('analog_read',"+value_io+")";
  return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['inout_highlow'] = function(block) {
  var dropdown_input = parseFloat(block.getFieldValue('input'));
  // TODO: Assemble JavaScript into code variable.
  var code=""+dropdown_input;
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['move_front'] = function(block) {
  var value_name = Blockly.JavaScript.valueToCode(block, 'NAME', Blockly.JavaScript.ORDER_ATOMIC);
  var dropdown_input = block.getFieldValue('input');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.move("+dropdown_input+","+value_name+");\n";
  return code;
};

Blockly.JavaScript['move_right'] = function(block) {
  var value_name = Blockly.JavaScript.valueToCode(block, 'NAME', Blockly.JavaScript.ORDER_ATOMIC);
  var dropdown_input = block.getFieldValue('input');
  // TODO: Assemble JavaScript into code variable.
  var code = "Blockly.turn("+dropdown_input+","+value_name+");\n";
  return code;
};
//