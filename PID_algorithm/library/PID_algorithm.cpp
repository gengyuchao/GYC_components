
/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-07-24).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */


#include "PID_algorithm.hpp"

// 控制算法  control algorithm.c





/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID_Control::SetTunings(PID_data_t new_Kp, PID_data_t new_Ki, PID_data_t new_Kd)
{
   if (new_Kp<0 || new_Ki<0 || new_Kd<0) return;

   Kp=new_Kp;
   Ki=new_Ki;
   Kd=new_Kd;
   
}



/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID_Control::SetOutputLimits(PID_data_t Min, PID_data_t Max)
{
   	if(Min >= Max) return;
	Min_out = Min;
   	Max_out = Max;

}


/* Status Funcions*************************************************************
* Just because you set the Kp=-1 doesn't mean it actually happened.  these
* functions query the internal state of the PID.  they're here for display
* purposes.  this are the functions the PID Front-end uses for example
******************************************************************************/
PID_data_t PID_Control::GetKp(){ return  Kp; }
PID_data_t PID_Control::GetKi(){ return  Ki; }
PID_data_t PID_Control::GetKd(){ return  Kd; }



/*
 * 执行PID计算
 */
void PID_Control::Compute()
{
	if(control_mode == Positional_Mode)
	{
		Positional_PID();
	}
	else if(control_mode == Incremental_Mode)
	{
		Incremental_PID();
	}
	else
	{
		printf("control_mode error!\n");
	}
}

/*
 * 设置目标值
 */
void PID_Control::Set_Target_Value(PID_data_t Target_value)
{
	expect = Target_value;
}

/*
 * 位置式PID算法
 */
PID_data_t PID_Control::Positional_PID()
{	

	Error[T_Currect] = expect - actual;

	if(esum>-Max_esum&&esum<Max_esum)
		esum += Error[T_Currect];
	
	output=0;
	
	output += Kp * Error[T_Currect];
	output += Ki * esum;
	output += Kd * (Error[T_Currect] - Error[T_Last]);

	Error[T_Last] = Error[T_Currect];
	
	if(output > Max_out) output = Max_out;
	if(output < Min_out) output = Min_out;

	return output;
}


/*
 * 增量式PID算法
 */
PID_data_t PID_Control::Incremental_PID()
{	

	Error[T_Currect] = expect - actual;
	
	output += Ki * Error[T_Currect];
	output += Kp * (Error[T_Currect]-Error[T_Last]);
	output += Kd * (Error[T_Currect]-2*Error[T_Last]+Error[T_Previous]);
	
	Error[T_Previous] = Error[T_Last];
	Error[T_Last] = Error[T_Currect];

	if(output > Max_out) output = Max_out;
	if(output < Min_out) output = Min_out;
	
	return output;

}




