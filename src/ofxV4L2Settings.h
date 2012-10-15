/*
 * ofxV4L2Settings.h
 *
 *  Created on: 03/03/2012
 *      Author: arturo
 */

#ifndef OFXV4L2SETTINGS_H_
#define OFXV4L2SETTINGS_H_

#include "ofConstants.h"
#include <map>
#include <linux/videodev2.h>
#include "ofParameter.h"
#include "ofParameterGroup.h"

class ofxV4L2Settings {
public:
	ofxV4L2Settings();
	virtual ~ofxV4L2Settings();

	bool setup(string device);
	bool set(string name, int value);
	void parameterChanged(const void * sender, int & value);

	struct Control{
		Control(){
			id = -1;
		}
		Control(int fd, const struct v4l2_queryctrl & ctrl, const struct v4l2_control & c);

		int id;
		ofParameter<int> parameter;
		enum v4l2_ctrl_type  type;
		int		     step;
		int		     default_value;
		vector<string> menu_options;

		int operator=(int value){
			if(id!=-1)
				parameter = value;
			return parameter;
		}

		operator int(){
			return parameter;
		}
	};

	Control & operator[](string name){
		return controls[name];
	}

	map<string,Control> controls;
	ofParameterGroup parameters;

	static string LOG_NAME;

private:
	int fd;
};

#endif /* OFXV4L2SETTINGS_H_ */
