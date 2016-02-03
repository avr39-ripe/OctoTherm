const maxProg = 6;
const antiFrozen = 5; // targetTemp in antifrozen mode
//setPoint inc / dec granularity
const setPointInc = 0.5;
//Used in color map to determine min and max possible temperature
const maxc = 26;
const minc = 5;

var now = new Date();
var timenow = now.getHours() + (now.getMinutes() / 60);
var thermostats = {};
var days = {
    0: 'sun',
    1: 'mon',
    2: 'tue',
    3: 'wed',
    4: 'thu',
    5: 'fri',
    6: 'sat',
    7: 'sun'
};
//var today = days[now.getDay()];
var today = now.getDay();

//=================================================
// DATA
//=================================================

var visibleFlag = 1;
var setpoint = 21;
var unit ="&deg;C";
var statusMsg = false;
var connected = false;
var doingsave = false;

var currThermostat = 0;
var thermostat = {
	name: "Zone Name",
	active: 0,
	state: 1,
	temperature: "22.00",
	manual: 0,
    manualTargetTemp: 2100,
    targetTempDelta: 50
};

var schedule = {};

var day1 = [{
    s: 000,
    tt: 800
}, {
    s: 600,
    tt: 1800
}, {
    s: 900,
    tt: 1200
}, {
    s: 1200,
    tt: 1500
}, {
    s: 1700,
    tt: 1800
}, {
    s: 2100,
    tt: 800
}];


schedule['1'] = JSON.parse(JSON.stringify(day1));
schedule['2'] = JSON.parse(JSON.stringify(day1));
schedule['3'] = JSON.parse(JSON.stringify(day1));
schedule['4'] = JSON.parse(JSON.stringify(day1));
schedule['5'] = JSON.parse(JSON.stringify(day1));
schedule['6'] = JSON.parse(JSON.stringify(day1));
schedule['0'] = JSON.parse(JSON.stringify(day1));

// ================================================
// State variables
// ================================================
var key = 1;
var day = "1";
var mousedown = 0;
var slider_width = $(".slider").width();
var slider_height = $(".slider").height();
var changed = 0;

function updateclock() {
    now = new Date();
    timenow = now.getHours() + (now.getMinutes() / 60);
//    today = days[now.getDay()];
    today = now.getDay();
	
	checkVisibility();

    $("#datetime").html(days[today].toUpperCase() + " " + format_time(timenow));

    if (thermostat.manual == 1 && thermostat.active == 1) {
        setpoint = thermostat.manualTargetTemp;
        $("#zone-setpoint").html(setpoint.toFixed(1) + unit);
    }
			
    var current_key = 0;
    
    for (var z in schedule[today]) {
 //       if (schedule[today][z].s <= timenow && endTime(z) > timenow) {
        if ((timenow >= schedule[today][z].s) && (timenow < endTime(z))) {
            if (thermostat.manual == 0 && thermostat.active == 1) {
                setpoint = schedule[today][z].tt * 1;
                $("#zone-setpoint").html(setpoint.toFixed(1) + unit);
                current_key = z;
            }
        }

    }

    var sx = $(".slider[day=" + today + "]")[0].offsetLeft;
    var y = $(".slider[day=" + today + "]")[0].offsetTop;
    var x1 = sx + slider_width * (timenow / 24.0);
    var x2 = sx + slider_width * (schedule[today][current_key].s / 24.0);

    x2 = sx;
    $("#timemarker").css('top', y + "px");
    $("#timemarker").css('left', x2 + "px");
    $("#timemarker").css('width', (x1 - x2) + "px");

}

function setStatus(msg,dur,pri){	 // show msg on status bar
		if(statusMsg == true){return};
		statusMsg= true;
		if(pri>0){
			$("#statusView").toggleClass("statusViewAlert",true);
			$("#statusView").toggleClass("statusView",false);
		} else {
			$("#statusView").toggleClass("statusView",true);
			$("#statusView").toggleClass("statusViewAlert",false);
		}
		$("#statusView").show();
		$("#statusView").html(msg);
		dur = dur*1000;
		if(dur >0){
			setTimeout(function(){$("#statusView").hide(200);$("#statusView").html(""); statusMsg= false},dur)
		}
	}

function update() {

//	$(".zone-title").html(thermostat.name);
	$(".zone-temperature").html((Number(thermostat.temperature)).toFixed(1) + "&deg;C");
	
	if(thermostat.state) {
		$("#zone-setpoint").css("color", "#f00000");
	} else {
		$("#zone-setpoint").css("color", "#000000");
	}
	
	if (thermostat.active) {
		$("#thermostatState").html("ON");
		$("#thermostatState").css("background-color", "#ff9600");
	} else {
		$("#thermostatState").html("OFF");
		$("#thermostatState").css("background-color", "#555");
		setpoint = antiFrozen;
		$("#zone-setpoint").html(setpoint.toFixed(1) + unit);
	}
	
	if (thermostat.manual) {
		$(".thermostatmode").css("background-color", "#555");
		$("#manual_thermostat").css("background-color", "#ff9600");
		$("#scheduled_thermostat").css("background-color", "#555");
	} else {
		$(".thermostatmode").css("background-color", "#555");
		$("#manual_thermostat").css("background-color", "#555");
		$("#scheduled_thermostat").css("background-color", "#ff9600");
	}
}

// ============================================
// SCHEDULER

function draw_day_slider(day) {
    var out = "";
    var key = 0;
    for (var z in schedule[day]) {
        var left = (schedule[day][z].s / 24.0) * 100;
        var width = ((endTime(key) - schedule[day][z].s) / 24.0) * 100;
        var color = color_map(schedule[day][z].tt);

        out += "<div class='slider-segment' style='left:" + left + "%; width:" + width + "%; background-color:" + color + "' key=" + key + " title='" + schedule[day][z].tt + unit +"'></div>";

        if (key > 0) {
            out += "<div class='slider-button' style='left:" + left + "%;' key=" + key + "></div>";
        }
        key++;
    }
    out += "<div class='slider-label'>" + days[day].toUpperCase() + "</div>";
    $(".slider[day=" + day + "]").html(out);
}

$("body").on("mousedown", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").mouseup(function (e) {
    mousedown = 0;
    if (changed) {
//        save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    	ajaxSaveDaySchedule(day);
        changed = 0;
    }
});

$("body").on("mousemove", ".slider", function (e) {
    //if (mousedown && editmode == 'move') {
    if (mousedown) {
        day = $(this).attr('day');
        slider_update(e);
    }
});

$("body").on("touchstart", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").on("touchend", ".slider-button", function (e) {
    mousedown = 0;
    if (changed) {
//        save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    	ajaxSaveDaySchedule(day);
        changed = 0;
    }
});

$("body").on("touchmove", ".slider", function (e) {

    var event = window.event;
    e.pageX = event.touches[0].pageX;
//    if (mousedown && editmode == 'move') {
    if (mousedown) {
        day = $(this).attr('day');
        slider_update(e);
    }
});

$("body").on("click", ".slider-segment", function (e) {

    day = $(this).parent().attr("day");
    key = parseInt($(this).attr("key"),10);
//	if (editmode == 'move') {
        $("#slider-segment-temperature").val((schedule[day][key].tt * 1).toFixed(1));
        $("#slider-segment-start").val(format_time(schedule[day][key].s));
        $("#slider-segment-end").val(format_time(endTime(key)));
        $("#slider-segment-block").show();
        $("#slider-segment-block-movepos").hide();
//    }
});

function slider_update(e) {
    $("#slider-segment-block-movepos").show();
    $("#slider-segment-block").hide();

    if (key !== undefined) {;
        var x = e.pageX - $(".slider[day=" + day + "]")[0].offsetLeft;

        var prc = x / slider_width;
        var hour = prc * 24.0;
        hour = Math.round(hour / 0.25) * 0.25;

        	if (hour > schedule[day][key - 1].s && hour < endTime(key)) {
              schedule[day][key].s = hour;
              update_slider_ui(day, key);
              changed = 1;
            }

        $("#slider-segment-time").val(format_time(schedule[day][key].s));
    }
    // $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));


}

$("body").on("click", "#slider-segment-ok", function () {
	var nextIdx = +key + 1;
	
    schedule[day][key].tt = $("#slider-segment-temperature").val();
    var color = color_map(schedule[day][key].tt);
    $(".slider[day=" + day + "]").find(".slider-segment[key=" + key + "]").css("background-color", color).attr("title",schedule[day][key].tt);

    var time = decode_time($("#slider-segment-start").val());
    if (time != -1 && key > 0 && key < schedule[day].length) {
	  	if (time >= (schedule[day][key - 1].s + 0.5) && time <= (endTime(key) - 0.5)) {
	          schedule[day][key].s = time;
	    }
    }

    $("#slider-segment-start").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);

    time = decode_time($("#slider-segment-end").val());
    if (time != -1 && key < (schedule[day].length - 1)) {
		if (time >= (schedule[day][key].s + 0.5) && time <= ((((nextIdx + 1) > maxProg - 1) ? 24 : schedule[day][nextIdx + 1].s) - 0.5)) {
			schedule[day][key + 1].s = time;
		}    
    }
    
    $("#slider-segment-end").val(format_time(endTime(key)));
    update_slider_ui(day, key + 1);
    
//    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    ajaxSaveDaySchedule(day);
    updateclock();

});

$("#slider-segment-movepos-ok").click(function () {
    var time = decode_time($("#slider-segment-time").val());

        if (time != -1 && key > 0) {
          if (time >= (schedule[day][key - 1].s + 0.5) && time <= (endTime(key) - 0.5)) {
              schedule[day][key].s = time;
          }
      }
    $("#slider-segment-time").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);
//    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    ajaxSaveDaySchedule(day);
});

function color_map(temperature) {
    /*
    // http://www.particleincell.com/blog/2014/colormap/
    // rainbow short
    var f=(temperature-minc)/(maxc-minc);	//invert and group
	var a=(1-f)/0.25;	//invert and group
	var X=Math.floor(a);	//this is the integer part
	var Y=Math.floor(255*(a-X)); //fractional part from 0 to 255
	switch(X)
	{
		case 0: r=255;g=Y;b=0;break;
		case 1: r=255-Y;g=255;b=0;break;
		case 2: r=0;g=255;b=Y;break;
		case 3: r=0;g=255-Y;b=255;break;
		case 4: r=0;g=0;b=255;break;
	}
     
	*/
    var f = (temperature - minc) / (maxc - minc);
    var a = (1 - f);
    var Y = Math.floor(255 * a);
    r = 255;
    g = Y;
    b = 0;

    return "rgb(" + r + "," + g + "," + b + ")";
}

function update_slider_ui(day, key) {
    if (schedule[day] !== undefined && key < schedule[day].length) {
        var slider = $(".slider[day=" + day + "]");
        if (key > 0) {
            var width = ((schedule[day][key].s - schedule[day][key - 1].s) / 24.0) * 100;
            slider.find(".slider-segment[key=" + (key - 1) + "]").css("width", width + "%");
        }

        var left = (schedule[day][key].s / 24.0) * 100;
        var width = ((endTime(key) - schedule[day][key].s) / 24.0) * 100;
        
        slider.find(".slider-segment[key=" + key + "]").css("width", width + "%");
        slider.find(".slider-segment[key=" + key + "]").css("left", left + "%");
        slider.find(".slider-button[key=" + key + "]").css("left", left + "%");
    }
}

function endTime(key) {
	var nextIdx = +key + 1;
	return (nextIdx == maxProg ? 24 : schedule[day][nextIdx].s);
}

function format_time(time) {
    var hour = Math.floor(time);
    var mins = Math.round((time - hour) * 60);
    if (mins < 10) mins = "0" + mins;
    return hour + ":" + mins;
}

function decode_time(timestring) {
    var time = -1;
    if (timestring.indexOf(":") != -1) {
        var parts = timestring.split(":");
        var hour = parseInt(parts[0],10);
        var mins = parseInt(parts[1],10);

        if (mins >= 0 && mins < 60 && hour >= 0 && hour < 25) {
            if (hour == 24 && mins !== 0) {} else {
                time = hour + (mins / 60);
            }
        }
    }
    return time;
}

function calc_average_schedule_temperature() {
    var sum = 0;
    for (var d in schedule) {
        for (var z in schedule[d]) {
        	var nextIdx = parseInt(z) < (maxProg - 1) ? parseInt(z) + 1 : 0;
//            var hours = (schedule[d][z].e - schedule[d][z].s)
            var hours = (schedule[d][nextIdx].s - schedule[d][z].s)
            sum += (schedule[d][z].tt * hours);
        }
    }
    return sum / (24 * 7.0);
}

function calc_schedule_esp(sched) {
    var fixsched = JSON.parse(JSON.stringify(sched));
    for (var d in fixsched) {
        fixsched[d].s *= 100;
 //       fixsched[d].e *= 100;
        fixsched[d].tt *= 100;
    }
    return fixsched;
}

function scheduleToFloat() {
	for (var d in schedule) {
	    for (var z in schedule[d]) {
	        schedule[d][z].s /= 100;
	        schedule[d][z].tt /= 100;
	    }
	}
}

// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
    $(window).bind("focus", function(event) {
        visibleFlag = 1;
    });

    $(window).bind("blur", function(event) {
        visibleFlag = 0;
    });
}
function toMinutes(hours) { //time in hours+DECIMAL minutes
	 var hour = Math.floor(hours);
	 var mins = Math.round((hours - hour) * 60);
 
	 return hour * 60 + mins;
}

function toHours(minutes) { //time in minutes
	 var hour = Math.floor(minutes / 60) ;
	 var mins = Math.round(minutes - hour * 60) / 60;

	 return hour + mins;
}

function loadDaySchedule(dayScheduleJson) {
	daySchedule = JSON.parse(dayScheduleJson);
	var day = Object.keys(daySchedule)[0];
	if (schedule[day] !== undefined) {
		var scheduleHours = JSON.parse(JSON.stringify(daySchedule[day]));
		for (var z in scheduleHours) {
			scheduleHours[z].s = toHours(scheduleHours[z].s);
		};
		schedule[day] = JSON.parse(JSON.stringify(scheduleHours));
	}
}

function save(param, payload) {
	doingsave=true;
    $.ajax({
        type: 'POST',
        url: "thermostat.cgi?param=" + param,
        data: payload,
		dataType: 'text',
		cache: false,
        async: true,
			timeout: 3000,
			tryCount : 0,
			retryLimit : 3,			success: function (data) {
			statusMsg = false;
			if(!connected) setStatus("Connected",2,0); 
			connected=true;
			doingsave=false;
		},
		error : function(xhr, textStatus, errorThrown ) {
        if (textStatus == 'timeout') {
            this.tryCount++;
            if (this.tryCount <= this.retryLimit) {
                //try again
                $.ajax(this);
                return;
            }            
            return;
        }
		if(connected) setStatus("No connection to server!",0,1);
		connected=false;
		doingsave=false;
		}
    });
}

function server_get() {
    var output = {};
	if (visibleFlag) {
		$.ajax({
			url: "thermostat.cgi?param=state",
			dataType: 'json',
			async: true,
			timeout: 3000,
			tryCount : 0,
			retryLimit : 3,				success: function (data) {
				if (data.length !== 0) {
					statusMsg = false;
					if(!connected) setStatus("Connected",2,0); 
					connected=true;
					if(!doingsave) {
						output = data;
						thermostat=data;
						thermostat.manualTargetTemp/=100;
						update();
					}
				}
			},
		error : function(xhr, textStatus, errorThrown ) {
        if (textStatus == 'timeout') {
            this.tryCount++;
            if (this.tryCount <= this.retryLimit) {
                //try again
                $.ajax(this);
                return;
            }            
            return;
        }
		if(connected) setStatus("No connection to server!",0,1);
		connected=false;
		}
		});
	}
    return output;
}

function server_get2(param) {
    var output = {};
	if (visibleFlag) {
		$.ajax({
			url: "thermostat.cgi?param=" + param,
			dataType: 'json',
			async: false,
			timeout: 3000,
			tryCount : 0,
			retryLimit : 3,			
			success: function (data) {
				if (data.length !== 0) output = data;
					statusMsg = false;
					if(!connected) setStatus("Connected",2,0); 
					connected=true;
			},
		error : function(xhr, textStatus, errorThrown ) {
        if (textStatus == 'timeout') {
            this.tryCount++;
            if (this.tryCount <= this.retryLimit) {
                //try again
                $.ajax(this);
                return;
            }            
            return;
        }
		if(connected) setStatus("No connection to server!",0,1);
		connected=false;
		}
		});
	}
    return output;
}

//PRE REFRACTORED CODE

//Here we put refractored code

function onThermostatStateButton() {
	thermostat.active ^= 1;
	thermostatStateButton = document.getElementById('thermostatState');
    if (thermostat.active == 1) {
    	thermostatStateButton.innerHTML = "ON";
    	thermostatStateButton.style.backgroundColor = "#ff9600";
    }
    if (thermostat.active === 0) {
    	thermostatStateButton.innerHTML = "OFF";
    	thermostatStateButton.style.backgroundColor = "#555";
    }

	ajaxSaveState("active");
    //save("tx/heating",thermostat.state+","+parseInt(setpoint*100));
//    save("thermostat_state", thermostat.active.toString());
}

function onSetPointDec(){
	if (thermostat.active == 1) {
		thermostat.manual = 1;
	    thermostat.manualTargetTemp -= setPointInc;
	    setpoint = thermostat.manualTargetTemp;
	    	
	    document.getElementById('scheduled_thermostat').style.backgroundColor = "#555";
		document.getElementById('manual_thermostat').style.backgroundColor = "#ff9600";
		document.getElementById('zone-setpoint').innerHTML = setpoint.toFixed(1) + unit;
	
		ajaxSaveState("manual");
		ajaxSaveState("manualTargetTemp");
//	save("thermostat_mode", thermostat.manual.toString());
//    save("thermostat_manualsetpoint", ((thermostat.manualTargetTemp.toFixed(1)) * 100).toString());
	}
}

function onSetPointInc(){
	if (thermostat.active == 1) {
	    thermostat.manual = 1;
	    thermostat.manualTargetTemp += setPointInc;
	    setpoint = thermostat.manualTargetTemp;
	    
		document.getElementById('scheduled_thermostat').style.backgroundColor = "#555";
		document.getElementById('manual_thermostat').style.backgroundColor = "#ff9600";
		document.getElementById('zone-setpoint').innerHTML = setpoint.toFixed(1) + unit;
		
		ajaxSaveState("manual");
		ajaxSaveState("manualTargetTemp");
//	save("thermostat_mode", thermostat.manual.toString());
//    save("thermostat_manualsetpoint", ((thermostat.manualTargetTemp.toFixed(1)) * 100).toString());
	}
}

function onManualThermostat() {
	if (thermostat.active == 1) {
		thermostat.manual = 1;
		setpoint = thermostat.manualTargetTemp;
		
		document.getElementById('scheduled_thermostat').style.backgroundColor = "#555";
		document.getElementById('manual_thermostat').style.backgroundColor = "#ff9600";
		document.getElementById('zone-setpoint').innerHTML = setpoint.toFixed(1) + unit;
		
	//	save("thermostat_mode", (thermostat.manual).toString());
		ajaxSaveState("manual");
	    updateclock();
	}
}

function onScheduledThermostat() {
	if (thermostat.active == 1) {
		thermostat.manual = 0;
		
		document.getElementById('manual_thermostat').style.backgroundColor = "#555";
		document.getElementById('scheduled_thermostat').style.backgroundColor = "#ff9600";
		
	//	save("thermostat_mode", (thermostat.manual).toString());
		ajaxSaveState("manual");
	    updateclock();
	}
}

function ajaxGetSchedule() {
	
	var xhr = new XMLHttpRequest();

	xhr.open('GET', '/schedule.json?thermostat=' + currThermostat, true);

	xhr.send();

	xhr.onreadystatechange = function() {
		
		if (this.readyState != 4) return;
		if (this.status == 200) {
			if (this.responseText.length > 0) {
				schedule = JSON.parse(this.responseText);
				for (var d in schedule) {
				    for (var z in schedule[d]) {
				        schedule[d][z].s = toHours(schedule[d][z].s);
				        schedule[d][z].tt /= 100;
				    }
				}
				for (day in schedule) draw_day_slider(day);
			}
		}
	  	else {
		    // обработать ошибку
	  		setStatus("No connection!",1,1);
		    return;
	    }
	}
}

function ajaxGetState() {
	
	var xhr = new XMLHttpRequest();

	xhr.open('GET', '/state.json?thermostat=' + currThermostat, true);

	xhr.send();

	xhr.onreadystatechange = function() {
		
		if (this.readyState != 4) return;
		if (this.status == 200) {
			if (this.responseText.length > 0) {
				var tstate = JSON.parse(this.responseText);
				thermostat.temperature = tstate.temperature;
				thermostat.state = tstate.state;
				thermostat.manual = tstate.manual;
				update();
			}
		}
	  	else {
		    // обработать ошибку
	  		setStatus("No connection!",1,1);
		    return;
	    }
	}
}

function ajaxGetAllState() {
	
	var xhr = new XMLHttpRequest();

	xhr.open('GET', '/state.json?thermostat=' + currThermostat, true);

	xhr.send();

	xhr.onreadystatechange = function() {
		
		if (this.readyState != 4) return;
		if (this.status == 200) {
			if (this.responseText.length > 0) {
				thermostat = JSON.parse(this.responseText);
				thermostat.manualTargetTemp /= 100;
				thermostat.targetTempDelta /= 100;
				update();
			}
		}
	  	else {
		    // обработать ошибку
	  		setStatus("No connection!",1,1);
		    return;
	    }
	}
}

function ajaxGetThermostats() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', '/thermostats.json', true);
	xhr.send();

	xhr.onreadystatechange = function() {
		if (this.readyState != 4) return;
		if (this.status == 200) {
			if (this.responseText.length > 0) {
				thermostats = JSON.parse(this.responseText);
				
				var select = document.getElementById("thermostats");
				Object.keys(thermostats).forEach(function(key) {
				        var newOption = new Option(thermostats[key], key);
				        select.appendChild(newOption);
				    });
			}
		}
	  	else {
		    // обработать ошибку
	  		setStatus("No connection!",1,1);
		    return;
	    }
	}
}

function onThermostats() {
	var select = document.getElementById("thermostats");
	currThermostat = select.options[select.selectedIndex].value;
	
	ajaxGetSchedule();
//	scheduleToFloat();
	
	ajaxGetAllState()
}

function ajaxSaveState(key) {
	
	var xhr = new XMLHttpRequest();

	xhr.open('POST', '/state.json?thermostat=' + currThermostat, true);
	xhr.setRequestHeader('Content-Type', 'application/json; charset=utf-8')
	
	var json = {};
	json[key] = thermostat[key];
	
	xhr.send(JSON.stringify(json));
}

function ajaxSaveDaySchedule(day) {
	if (schedule[day] !== undefined) {
		var xhr = new XMLHttpRequest();

		xhr.open('POST', '/schedule.json?thermostat=' + currThermostat, true);
		xhr.setRequestHeader('Content-Type', 'application/json; charset=utf-8')
		
		var scheduleMinutes = JSON.parse(JSON.stringify(schedule[day]));
		for (var z in scheduleMinutes) {
			scheduleMinutes[z].s = toMinutes(scheduleMinutes[z].s);
			scheduleMinutes[z].tt *= 100;
		};
		var json = {}
		json[day] = scheduleMinutes;
		
		xhr.send(JSON.stringify(json));
		}
}

//Here we put some initial code which starts after DOM loaded
function onDocumentRedy() {
	//Attach eventListeners
	document.getElementById('thermostats').addEventListener('change', onThermostats);
	document.getElementById('thermostatState').addEventListener('click', onThermostatStateButton);
	document.getElementById('zone-setpoint-dec').addEventListener('click', onSetPointDec);
	document.getElementById('zone-setpoint-inc').addEventListener('click', onSetPointInc);
	document.getElementById('manual_thermostat').addEventListener('click', onManualThermostat);
	document.getElementById('scheduled_thermostat').addEventListener('click', onScheduledThermostat);
	
	//Init
	//schedule = server_get2("thermostat_schedule"); //all data * 100 to avoid floating point on the ESP8266 side
	ajaxGetThermostats();
	
	ajaxGetSchedule();
	scheduleToFloat();
	
	ajaxGetAllState()
	
//	update();
//	updateclock();
	
	setInterval(ajaxGetState, 5000);
	setInterval(updateclock, 1000);
	
//	for (day in schedule) draw_day_slider(day);
}

document.addEventListener('DOMContentLoaded', onDocumentRedy);