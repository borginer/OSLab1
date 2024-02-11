#include <linux/kernel.h>
#include <linux/sched.h>

int sys_rpg_create_character(void){
    return 1;
}
int sys_rpg_fight(void){
    return 2;
}
int sys_rpg_get_stats(void){
    return 3;
}
int sys_rpg_join_party(void){
    return 4;
}