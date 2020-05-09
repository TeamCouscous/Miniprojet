#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//List of the turning states
enum{
	STOP,
	STRAIGHT,
	LEFT,
	RIGHT,
	BACK
};

//constants for the differents parts of the project
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_LINE_WIDTH			40
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2
#define SPEED_THRESHOLD			10
#define MOV_START				1
#define MOV_STOP				0
#define MOV_CONTINUE			2
#define PROXIMITY_MAX			50
#define MAX_COUNTER				100
#define MAX_SPEED				800
#define BACKWARD_SPEED			-200
#define G_COEFF					50
#define S_COEFF					20
#define BLINKING_THRESHOLD		20 //en pixels
#define GYRO_MIN				100
#define G_MIN					10

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
