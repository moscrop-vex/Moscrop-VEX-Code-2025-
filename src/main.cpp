#include "main.h"

#define GAME_CONTROLLER_STYLE 1

constexpr unsigned int INTAKE_SPEED = 50;

pros::Controller master(pros::E_CONTROLLER_MASTER);

pros::adi::DigitalOut piston('H');                      // ADI port of Solenoid
pros::MotorGroup left_mg({1, 2, 3});                    // L motor smart ports
pros::MotorGroup right_mg({4, 5, 6});                   // R motor smart ports

pros::MotorGroup intake_mg({8, 9});

bool piston_state = false;                              // true = extended
int leftmove, rightmove;                                // Motor angular offset (?)
bool toggle = false;

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
        /*
        static bool pressed = false;
        pressed = !pressed;
        if (pressed) {
                pros::lcd::set_text(2, "I was pressed!");
        } else {
                pros::lcd::clear_line(2);
        }*/
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competitiocd::clear_line(2);
        }n modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

void initialize() {
        pros::lcd::initialize();
        // pros::lcd::register_btn1_cb(on_center_button);
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
void autonomous() {}

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

void get_controller_input(int *leftmove, int *rightmove) {
        constexpr float accel_offset = 1;
        #if GAME_CONTROLLER_STYLE
        *leftmove = -master.get_analog(ANALOG_LEFT_Y);           // Gets amount forward/backward from left joystick
        *rightmove = master.get_analog(ANALOG_RIGHT_Y);          // Gets the turn left/right from right joystick
        #else
        int vel = master.get_analog(ANALOG_LEFT_Y);
        int rad = master.get_analog(ANALOG_RIGHT_X);

        *leftmove = vel + rad * accel_offset;
        *rightmove = vel - rad * accel_offset;
        #endif
}

void opcontrol() {
        constexpr int loop_delay = 20;

        while (true) {
                if (master.get_digital(DIGITAL_R2) && master.get_digital(DIGITAL_L2)) {
                        if (!toggle) {
                                piston_state = !piston_state;
                                piston.set_value(piston_state);
                        }
                        toggle = true;
                } else {
                        toggle = false;
                }

                intake_mg.move(master.get_digital(DIGITAL_A) * INTAKE_SPEED);

                left_mg.move(leftmove);                                 // Sets left motor voltage
                right_mg.move(rightmove);                               // Sets right motor voltage

                pros::delay(loop_delay);                                // Run for ``loop_delay`` ms then update
        }
}
