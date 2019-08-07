
/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-07-24).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef __PID_ALGORITHM__
#define __PID_ALGORITHM__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "string.h"
#include "unistd.h"


#ifndef PID_Need_Double_Data_Type 
typedef float PID_data_t;
#else
typedef double PID_data_t;
#endif

typedef enum {
	Positional_Mode 	= 0,
	Incremental_Mode 	= 1
} PID_Mode;

class PID_Control
{
	public:
		
		PID_Control(PID_data_t _Kp,PID_data_t _ki,PID_data_t _Kd, PID_data_t min_output,PID_data_t max_output, PID_Mode PID_mode=Positional_Mode)
		{
			SetTunings(_Kp, _ki, _Kd);
			SetOutputLimits(min_output,max_output);
			control_mode = PID_mode;
		}
		enum timeline_t
		{
			T_Previous 	= 0,	//上上次偏差
			T_Last 		= 1,	//上一次偏差
			T_Currect 	= 2,	//当前偏差
			T_Max		= 3		//最大时间数
		};


		
		PID_data_t GetKp();
		PID_data_t GetKi();
		PID_data_t GetKd();

		void SetTunings(PID_data_t new_Kp, PID_data_t new_Ki, PID_data_t new_Kd);
		void SetOutputLimits(PID_data_t Min, PID_data_t Max);

		void Set_Target_Value(PID_data_t Target_value);

		void Compute();
	

		PID_data_t expect;
		PID_data_t actual;
		PID_data_t output;	

	private:
		PID_data_t Error[T_Max]; //偏差记录数组
		PID_data_t esum;		 //偏差积分
		PID_data_t Max_esum;	 //积分限额
		PID_data_t Kp;			 //比例项
		PID_data_t Ki;			 //积分项
		PID_data_t Kd;			 //微分项
		PID_data_t Max_out;		 //最大输出值
		PID_data_t Min_out;		 //最小输出值

		//PID 控制方式 位置式 or 增量式
		PID_Mode control_mode=Positional_Mode;

		PID_data_t Positional_PID();
		PID_data_t Incremental_PID();
};




#endif
