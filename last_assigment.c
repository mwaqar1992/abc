
#include <stdio.h>
#include <thread.h>
#include <xtimer.h>
#include <periph/gpio.h>
#include <mutex.h>
#include <msg.h>
kernel_pid_t Scan_Inputs;
kernel_pid_t Compute_AND_Gate;
kernel_pid_t Result_of_AND_Gate;
mutex_t lock;
gpio_t First_Input_Pin = GPIO_PIN(PORT_B,5); // Aurduino pin 11
gpio_t Second_Input_Pin = GPIO_PIN(PORT_B,6); // Aurduino pin 12
gpio_t Output_pin = GPIO_PIN(PORT_B,7); //Aurduino pin 13
void * Scan_AND_Inputs(void* arg)
{
(void)arg;
gpio_init(First_Input_Pin,GPIO_IN_PU);
gpio_init(Second_Input_Pin,GPIO_IN_PU);
int AND_Gate_Inputs[2];
msg_t Incoming_message, Outgoing_message;
while(1)
{
msg_receive(&Incoming_message); 
mutex_lock (&lock); 
if(gpio_read(First_Input_Pin) == 0)
AND_Gate_Inputs[0] = 0;
else AND_Gate_Inputs[0] = 1;
if(gpio_read(Second_Input_Pin) == 0)
AND_Gate_Inputs[1] = 0;
else AND_Gate_Inputs[1] = 1;
Outgoing_message.content.ptr=AND_Gate_Inputs;
mutex_unlock (&lock); 
xtimer_usleep(1000); 
msg_send(&Outgoing_message, Compute_AND_Gate); 
}
}
void * Processing_AND(void* arg)
{
(void)arg;
msg_t Incoming_message, Outgoing_message;
while(1)
{
msg_receive(&Incoming_message);
mutex_lock (&lock);
Outgoing_message.content.value = *(int *)Incoming_message.content.ptr & *((int*)Incoming_message.content.ptr+1); 

mutex_unlock (&lock);
xtimer_usleep(250); 
msg_send(&Outgoing_message, Result_of_AND_Gate); 
}
}
void * AND_Gate_Output(void* arg)
{
(void)arg;

msg_t Incoming_message;
gpio_init(Output_pin,GPIO_OUT); 
while(1)
{
msg_receive(&Incoming_message); 
mutex_lock (&lock);
if(Incoming_message.content.value == 1)
gpio_set(Output_pin);
else
gpio_clear(Output_pin);
mutex_unlock (&lock);
xtimer_usleep(1000);
}
}
char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
int main(void)
{
msg_t Outgoing_message;
mutex_init(&lock); 
Scan_Inputs = thread_create (t1_stack, sizeof(t1_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, Scan_AND_Inputs, NULL, NULL);
Compute_AND_Gate = thread_create (t2_stack, sizeof(t2_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, Processing_AND, NULL, NULL);
Result_of_AND_Gate = thread_create (t3_stack, sizeof(t3_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, AND_Gate_Output, NULL, NULL);
while (1) {
msg_send(&Outgoing_message, Scan_Inputs); 
}
return 0;
}



