/* 
 * File:   can_messages_def.h
 * Author: Savioz Benjamin
 *
 * Created on 15. mai 2019, 14:01
 */

#ifndef CAN_MESSAGES_DEF_H
#define	CAN_MESSAGES_DEF_H
#include <pic18.h>
#include <stdint.h>

//From Car to controller
#define TEMPO 0x1
#define GEAR_SEL 0x2
#define SENSOR 0x3
#define MOTOR_STATUS 0x4
#define BREAK_PEDAL 0x6
#define ACCEL_PEDAL 0x7
#define CONTACT_KEY 0x8
#define STEERING_W_REQ 0x9
#define BROKEN_CAR 0xA
#define BAD_MSG 0xB
#define SLOPE_REQ 0xC
#define RACE 0xD
#define REV 0xFF

//From controller to car
#define LIGHT_FRONT 0x11
#define LIGHT_BACK 0x12
#define TIME 0x13
#define GEAR_LVL 0x14
#define AUDIO 0x15
#define PWR_MOTOR 0x16
#define PWR_BRAKE 0x17
#define TEMPO_OFF 0x18
#define KM_PULSE 0x19
#define AUTO_STEERING 0x1A
#define CAR_RST 0x1F

//CAR car to controller
struct CAR{
	uint8_t tempomat_state;
	uint8_t tempomat_speed;
	uint8_t gear_sel;
    uint8_t gear_lvl;
	uint16_t front_sens_req;
	uint8_t ext_sensorL;
	uint8_t ext_sensorR;
    uint16_t motor_rpm;
    int16_t motor_speed;        //signed value, can go backward
	uint8_t break_pedal;
    uint16_t accel_pedal;
	uint8_t contact_key;
	int8_t steering_w_req;      //signed value, +/-100
	uint8_t broken_car;	
	uint8_t bad_message;
	int8_t slope_req;           //signed value, +/-100
	uint8_t race;
	uint8_t rev;
};

//Extern for classes, include at least the .h
extern struct CAR car;

#ifdef	__cplusplus
extern "C" {
#endif
    
#ifdef	__cplusplus
}
#endif
//read message function
void messageEndling(struct CANMESSAGE * msg);  

//send message functions
void setLightFront(uint8_t intensity);
void setLightBack(uint8_t intensity);
void setTime(uint8_t hours, uint8_t minutes, uint8_t visibility);
void setGearLevel(uint8_t glvl);
void setAudio(uint8_t audioLvl, uint8_t wDrive);
void setPwrMotor(uint8_t power);
void setPwrBrake(uint8_t bFactor);
void setTempoOff();
void setKmPulse();
void setAutoSteering(uint8_t wheelPos, uint8_t autoControlSet);
void setCarRst();

#endif	/* CAN_MESSAGES_DEF_H */

