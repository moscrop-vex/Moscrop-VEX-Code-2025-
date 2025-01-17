#include "main.h"

#define GAME_CONTROLLER_STYLE 1                         // 0 = game controller; 1 = hemisphere
constexpr int INTAKE_SPEED = -80;
constexpr int AUTONOMOUS_STAGE_RUNTIME = 1000;

pros::Controller master(pros::E_CONTROLLER_MASTER);     // Sets controller

pros::adi::DigitalOut piston('H');                      // ADI port of Solenoid
pros::MotorGroup left_mg({1, 2, 3});                    // L motor smart ports
pros::MotorGroup right_mg({4, 5, 6});                   // R motor smart ports

pros::MotorGroup intake_mg({8});

bool piston_state = false;                              // true = extended
int leftmove, rightmove;                                // Motor angular offset (?)
bool toggle = false;                                    // Auxiliary variable for debouncing piston actuation


/**
 * Auxiliary functions
 */
int curve_velocity(int velocity) {
        /* Maps linear ouput to cubic curve */
        int curve = (velocity * velocity * velocity) / (127 * 127);

        return curve;
}

int abs(int x) {
        return (x < 0) ? -x : x;
}

int sign(int x) {
        return (x < 0) ? -1 : 1;
}

void get_controller_input(int *leftmove, int *rightmove) {
        #if GAME_CONTROLLER_STYLE
        *leftmove = -master.get_analog(ANALOG_LEFT_Y);           // Gets amount forward/backward from left joystick
        *rightmove = master.get_analog(ANALOG_RIGHT_Y);          // Gets the turn left/right from right joystick
        #else
        constexpr float ACCEL_OFFSET = 0.3;                             // Sets modifier for how much to turn

        int vel = master.get_analog(ANALOG_LEFT_Y);
        int rad = master.get_analog(ANALOG_RIGHT_X);

        *leftmove = -(vel + rad * ACCEL_OFFSET); // Get velocity based on l/r joysticks
        *rightmove = vel - rad * ACCEL_OFFSET;


        // Code to avoid exceeding 127 in analog motor input
        if (abs(*leftmove) > 127) {
                *rightmove = (*rightmove * 127) / abs(*leftmove);
                *leftmove = 127 * sign(*leftmove);
        }

        if (abs(*rightmove) > 127) {
                *leftmove = (*leftmove * 127) / abs(*rightmove);
                *rightmove = 127 * sign(*rightmove);
        }

        #endif
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

void initialize() {
        pros::lcd::initialize();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

void autonomous() {
        constexpr float ratio = 20.42035224;

        leftmove = 127;
        rightmove = -127;
        left_mg.move(leftmove * ratio);
        right_mg.move(rightmove * ratio);

        pros::delay(AUTONOMOUS_STAGE_RUNTIME);
        left_mg.move(0);
        right_mg.move(0);
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */

void opcontrol() {
        constexpr int LOOP_DELAY = 20;

        while (true) {
                if (master.get_digital(DIGITAL_R2) && master.get_digital(DIGITAL_L2) && !toggle) { // Only if input has changed
                        piston_state = !piston_state;
                        piston.set_value(piston_state);
                }

                toggle = master.get_digital(DIGITAL_R2) && master.get_digital(DIGITAL_L2);

                if (master.get_digital(DIGITAL_A)) {                                    // Sets reverse, forward, motionless
                        intake_mg.move(INTAKE_SPEED);
                } else if (master.get_digital(DIGITAL_B)) {
                        intake_mg.move(-INTAKE_SPEED);
                } else {
                        intake_mg.move(0);
                }

                get_controller_input(&leftmove, &rightmove);

                left_mg.move(curve_velocity(leftmove));                                 // Sets left motor voltage
                right_mg.move(curve_velocity(rightmove));                               // Sets right motor voltage

                pros::delay(LOOP_DELAY);                                // Run for ``loop_delay`` ms then update
        }
}
