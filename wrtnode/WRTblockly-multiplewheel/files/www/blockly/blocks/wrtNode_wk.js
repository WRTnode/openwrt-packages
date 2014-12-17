/**
 * @the block for wrtNOde
 * @author wankge90@qq.com(wangke)
 */
'use strict';
var profile = {
	wrt: {
		description: "Arduino standard-compatible board",
		digital : [["0", "0"],["1", "1"], ["2", "2"], ["3", "3"], ["4", "4"], ["5", "5"], ["6", "6"], ["7", "7"], ["8", "8"], ["9", "9"], ["10", "10"], ["11", "11"], ["12", "12"], ["13", "13"], ["A0", "14"], ["A1", "15"], ["A2", "16"], ["A3", "17"], ["A4", "18"], ["A5", "19"]],
		analog : [["A0", "14"], ["A1", "15"], ["A2", "16"], ["A3", "17"], ["A4", "18"], ["A5", "19"]],
		pwm : [["5", "5"], ["6", "6"], ["7", "7"], ["13", "13"]],
        serial : 115200,
	},
}
//set default profile to arduino standard-compatible board
profile["default"] = profile["wrt"];


Blockly.Blocks['wrtnode_temp'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(230);
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("http://www.gstatic.com/codesite/ph/images/star_on.gif", 15, 15, "*"))
        .appendField("温度");
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};
Blockly.Blocks['wrtnode_humi'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(230);
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("http://www.gstatic.com/codesite/ph/images/star_on.gif", 15, 15, "*"))
        .appendField("湿度");
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrtnode_pres'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(230);
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("http://www.gstatic.com/codesite/ph/images/star_on.gif", 15, 15, "*"))
        .appendField("气压");
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};
Blockly.Blocks['wrtnode_dist'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(315);
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("http://www.gstatic.com/codesite/ph/images/star_on.gif", 15, 15, "*"))
        .appendField("超声波测距");
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};
Blockly.Blocks['wrtnode_init'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(195);
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("http://www.gstatic.com/codesite/ph/images/star_on.gif", 15, 15, "*"))
        .appendField("插入设备初始化");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_high'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendDummyInput()
        .appendField("高电平");
    this.setOutput(true, "Boolean");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_low'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("低电平");
    this.setOutput(true, "Boolean");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_output1'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("拐角1");
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_13output'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("设置13口输出");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_13outputhigh'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("13端口输出高电平");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_13outputlow'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("13端口输出低电平");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_13outpwm'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(180);
    this.appendDummyInput()
        .appendField("13端口呼吸灯测试");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

//To support syntax defined in http://arduino.cc/en/Reference/HomePage

//define blocks

Blockly.Blocks['base_delay'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendValueInput("time")
        .setCheck("Number")
        .appendField("延时");
    this.appendDummyInput()
        .appendField("ms");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};



Blockly.Blocks['inout_digital_write'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendDummyInput()
        .appendField("设置端口")
        .appendField(new Blockly.FieldDropdown(profile.default.digital), "io")
        .appendField("为")
        .appendField(new Blockly.FieldDropdown([["高", "1"], ["低", "0"]]), "input")
        .appendField("电平");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['inout_digital_read'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendDummyInput()
        .appendField("高低电压读取")
        .appendField(new Blockly.FieldDropdown(profile.default.digital), "io")
		.appendField("端口");
    this.setInputsInline(true);
    this.setOutput(true, "Boolean");
    this.setTooltip('');
  }
};

Blockly.Blocks['inout_analog_write'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendValueInput("v")
        .setCheck("Number")
        .appendField("模拟输出");
    this.appendDummyInput()
        .appendField("v电压到端口")
        .appendField(new Blockly.FieldDropdown(profile.default.analog), "io");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['inout_analog_read'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendDummyInput()
        .appendField("模拟电压读取")
        .appendField(new Blockly.FieldDropdown(profile.default.analog), "io")
        .appendField("端口");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};

Blockly.Blocks['inout_highlow'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(270);
    this.appendDummyInput()
        .appendField(new Blockly.FieldDropdown([["高", "1"], ["低", "0"]]), "input");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};



Blockly.Blocks['inout_dialog_freq'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(290);
    this.appendDummyInput()
        .appendField("端口")
        .appendField(new Blockly.FieldDropdown(profile.default.pwm), "io");
    this.appendValueInput("v")
        .setCheck("Number")
        .appendField("波特率");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['inout_dialog_cover'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(290);
    this.appendDummyInput()
        .appendField("端口")
        .appendField(new Blockly.FieldDropdown(profile.default.pwm), "io");
    this.appendValueInput("v")
        .setCheck("Number")
        .appendField("占空比");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};
Blockly.Blocks['inout_dialog_freqcover'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(65);
    this.appendDummyInput()
        .appendField("端口")
        .appendField(new Blockly.FieldDropdown(profile.default.pwm), "io");
    this.appendValueInput("fre")
        .setCheck("Number")
        .appendField("波特率");
    this.appendValueInput("cov")
        .setCheck("Number")
        .appendField("占空比");
    this.setInputsInline(true);
	this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_iosetgpio'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(260);
    this.appendDummyInput()
        .appendField("设置")
        .appendField(new Blockly.FieldDropdown(profile.default.digital), "io")
        .appendField("端口为标准")
        .appendField(new Blockly.FieldDropdown([["输出", "1"], ["输入", "0"]]), "type");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};


Blockly.Blocks['wrt_dunio_ioset'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendDummyInput()
        .appendField("设置")
        .appendField(new Blockly.FieldDropdown(profile.default.digital), "io")
        .appendField("端口")
        .appendField(new Blockly.FieldDropdown([["标准IO", "0"], ["脉宽调制", "1"], ["模拟输出", "2"]]), "type");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_iosetmn'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendDummyInput()
        .appendField("设置")
        .appendField(new Blockly.FieldDropdown(profile.default.analog), "io")
        .appendField("端口模拟电压读入模式");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_setpwm'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendValueInput("io")
        .setCheck("Number")
        .appendField("端口");
    this.appendValueInput("rate")
        .setCheck("Number")
        .appendField("波特率");
    this.appendValueInput("cover")
        .setCheck("Number")
        .appendField("占空比");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_setiogene'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(260);
    this.appendValueInput("NAME")
        .setCheck("Number")
        .appendField("设置");
    this.appendDummyInput()
        .appendField("端口为标准")
        .appendField(new Blockly.FieldDropdown([["输出", "1"], ["输入", "0"]]), "type");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};


Blockly.Blocks['wrt_dunio_setioout'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendValueInput("io")
        .setCheck("Number")
        .appendField("设置端口");
    this.appendValueInput("num")
        .setCheck("Number")
        .appendField("为");
    this.appendDummyInput()
        .appendField("电平");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_setioreada'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendValueInput("io")
        .setCheck("Number")
        .appendField("设置");
    this.appendDummyInput()
        .appendField("端口模拟电压读入模式");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_readiod'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendValueInput("io")
        .setCheck("Number")
        .appendField("高低电压");
    this.appendDummyInput()
        .appendField("端口模拟电压读入");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};

Blockly.Blocks['wrt_dunio_readioa'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(0);
    this.appendValueInput("io")
        .setCheck("Number")
        .appendField("模拟电压");
    this.appendDummyInput()
        .appendField("端口模拟电压读入");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setTooltip('');
  }
};

Blockly.Blocks['move_front'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(195);
    this.appendValueInput("NAME")
        .setCheck("Number")
        .appendField(new Blockly.FieldDropdown([["前移指定格数", "1"], ["后移指定格数", "0"]]), "input");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};

Blockly.Blocks['move_right'] = {
  init: function() {
    this.setHelpUrl('http://www.example.com/');
    this.setColour(195);
    this.appendValueInput("NAME")
        .setCheck("Number")
        .appendField(new Blockly.FieldDropdown([["左转", "0"], ["右转", "1"]]), "input");
    this.setInputsInline(true);
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
  }
};
// define generators
