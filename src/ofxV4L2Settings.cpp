/*
 * ofxV4L2Settings.cpp
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#include "ofxV4L2Settings.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <libv4l2.h>
#include "ofLog.h"
string ofxV4L2Settings::LOG_NAME = "ofxV4L2Settings";


ofxV4L2Settings::Control::Control(int fd, const struct v4l2_queryctrl & ctrl, const struct v4l2_control & c){
	id = c.id;
	parameter.set((char*)ctrl.name,c.value,ctrl.minimum,ctrl.maximum);
	step = ctrl.step;
	type = (__u32)ctrl.type;
	default_value = ctrl.default_value;

	if(ctrl.type == V4L2_CTRL_TYPE_MENU){
		struct v4l2_querymenu menu;
		menu.id = c.id;
		ofLogVerbose(LOG_NAME) << "control menu for " << ctrl.name;
		for(int j=0;j<=ctrl.maximum;j++){
			menu.index = j;
			if(v4l2_ioctl(fd, VIDIOC_QUERYMENU, &menu)==0){
				ofLogVerbose(LOG_NAME) << "    " << j << ": " << menu.name;
				menu_options.push_back((char*)menu.name);
			}else{
				ofLogError(LOG_NAME) << "error couldn0t get menu option " << j<< strerror(errno);
			}
		}
	}
}

ofxV4L2Settings::ofxV4L2Settings() {
	fd=0;

}

ofxV4L2Settings::~ofxV4L2Settings() {
	if(fd) v4l2_close(fd);
}

bool ofxV4L2Settings::setup(string device){
    struct v4l2_queryctrl ctrl;
    struct v4l2_control c;
    parameters.setName("v4l2 " + device);

    fd = v4l2_open(device.c_str(), O_RDWR, 0);
	if(fd < 0) {
		ofLogError(LOG_NAME) <<  "Unable to open " << device.c_str() << " " << strerror(errno);
		return false;
	}



#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
    /* Try the extended control API first */
    ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    if(v4l2_ioctl (fd, VIDIOC_QUERYCTRL, &ctrl)==0) {
		do {
			c.id = ctrl.id;
			ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
			if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
				continue;
			}
			if(ctrl.type != V4L2_CTRL_TYPE_INTEGER &&
			   ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
			   ctrl.type != V4L2_CTRL_TYPE_MENU) {
				continue;
			}
			if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
				Control control(fd,ctrl,c);
				ofLogVerbose(LOG_NAME) << "adding " << control.parameter.getName();
				controls[control.parameter.getName()] = control;
				control.parameter.addListener(this,&ofxV4L2Settings::parameterChanged);
				parameters.add(control.parameter);
			}
		} while(v4l2_ioctl (fd, VIDIOC_QUERYCTRL, &ctrl)== 0);
    } else
#endif
    {
        /* Check all the standard controls */
        for(int i=V4L2_CID_BASE; i<V4L2_CID_LASTP1; i++) {
            ctrl.id = i;
            if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                    continue;
                }
                if(ctrl.type != V4L2_CTRL_TYPE_INTEGER &&
                   ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                   ctrl.type != V4L2_CTRL_TYPE_MENU) {
                    continue;
                }

                c.id = i;
                if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
    				Control control(fd,ctrl,c);
    				ofLogVerbose(LOG_NAME) << "adding " << control.parameter.getName();
    				controls[control.parameter.getName()] = control;
    				control.parameter.addListener(this,&ofxV4L2Settings::parameterChanged);
    				parameters.add(control.parameter);
                }
            }
        }

        /* Check any custom controls */
        for(int i=V4L2_CID_PRIVATE_BASE; ; i++) {
            ctrl.id = i;
            if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                    continue;
                }
                if(ctrl.type != V4L2_CTRL_TYPE_INTEGER &&
                   ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                   ctrl.type != V4L2_CTRL_TYPE_MENU) {
                    continue;
                }

                c.id = i;
                if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
    				Control control(fd,ctrl,c);
    				ofLogVerbose(LOG_NAME) << "adding " << control.parameter.getName();
    				controls[control.parameter.getName()] = control;
    				control.parameter.addListener(this,&ofxV4L2Settings::parameterChanged);
    				parameters.add(control.parameter);
                }
            } else {
                break;
            }
        }
    }

    return true;

}

bool ofxV4L2Settings::set(string name, int value){
	struct v4l2_queryctrl ctrl;
	struct v4l2_control c;

	if(controls.find(name)==controls.end()){
		ofLogError(LOG_NAME) << name << " not found";
		return false;
	}else{
		ctrl.id = controls[name].id;
	}
	if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
		if(strcmp((char *)ctrl.name, name.c_str())) {
			ofLogError() << "Control name mismatch " << name <<"!="<< ctrl.name;
			return false;
		}

		if(ctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY |
						 V4L2_CTRL_FLAG_DISABLED |
						 V4L2_CTRL_FLAG_GRABBED)) {
			ofLogError(LOG_NAME) << name << " not writable";
			return false;
		}
		if(ctrl.type != V4L2_CTRL_TYPE_INTEGER &&
		   ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
		   ctrl.type != V4L2_CTRL_TYPE_MENU) {
			ofLogError(LOG_NAME) << name << " type not supported";
			return false;
		}
		c.id = ctrl.id;
		c.value = value;
		if(v4l2_ioctl(fd, VIDIOC_S_CTRL, &c) != 0) {
			ofLogError(LOG_NAME) << "Failed to set control " << name << " error: " << strerror(errno);
			return false;
		}
	} else {

		ofLogError(LOG_NAME) << "Error querying control " << name << strerror(errno);
		return false;
	}

	return true;
}

void ofxV4L2Settings::parameterChanged(const void * sender,int & value){
	ofParameter<int> & param = *(ofParameter<int>*)sender;
	set(param.getName(),value);
	if(controls[param.getName()].type==V4L2_CTRL_TYPE_MENU && value<(int)controls[param.getName()].menu_options.size()){
		ofLogVerbose(LOG_NAME) << param.getName() << "=" << controls[param.getName()].menu_options[value];
	}else{
		ofLogVerbose(LOG_NAME) << param.getName() << "=" << value;
	}
}
