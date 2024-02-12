#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define FIGHTER 0
#define MAGE 1
#define ORC 0
#define DEMON 1

struct rpg_stats {
    int cclass;
    int level;
    int party_size;
    int fighter_levels;
    int mage_levels;
};

int sys_rpg_create_character(int cclass){
    if (cclass != 0 && cclass != 1) {
        return -EINVAL;
    } else if (current->character != NULL) {
        return -EEXIST;
    }

    struct rpg_character *character = vmalloc(sizeof(struct rpg_character));
    if (!character) {
        return -1;
    }
    struct list_head *party = vmalloc(sizeof(struct list_head));
    if (!party) {
        vfree(character);
        return -1;
    }

    INIT_LIST_HEAD(party);
    list_add(character->list, party);   
    character.cclass = cclass;
    character.level = 1;
    current->character = character;
    current->party = party;
    return 0;
}

int sys_rpg_fight(int type, int level){
    if ((type != 0 && type != 1) || (level < 0) || (current->character == NULL)) {
        return -EINVAL;
    }
    
    int sum_strength = 0;
    struct list_head *pos;
    struct rpg_character *entry;

    list_for_each(pos, current->party) {
        entry = list_entry(pos, struct rpg_character, list);
        if (entry->cclass == type) {
            sum_strength += 2 * entry->level;
        } else {
            sum_strength += 1 * entry->level;
        }
    }

    if (sum_strength >= level) {
        list_for_each(pos, current->party) {
            entry = list_entry(pos, struct rpg_character, list);
            entry->level++;
        }
    } else {
        list_for_each(pos, current->party) {
            entry = list_entry(pos, struct rpg_character, list);
            entry->level = (entry->level - 1 < 0) ? 0 : entry->level - 1;  
        }
    }
    
    return !!(sum_strength >= level);
}

int sys_rpg_get_stats(struct rpg_stats *stats){
    if (stats == NULL || current->character.party.prev == NULL) {
        return -EINVAL;
    }

    struct rpg_stats out;
    out.cclass = current->character.cclass;
    out.level = current->character.level;

    int party_size = 0;
    int fighter_levels = 0;
    int mage_levels = 0;

    struct list_head *pos;
    struct rpg_character *entry;

    list_for_each(pos, current->party) {
        entry = list_entry(pos, struct rpg_character, list);
        party_size++;
        if (entry->cclass == MAGE) {
            mage_levels += entry->level;
        } else {
            fighter_levels += entry->level;
        }
    }

    out.party_size = party_size;
    out.fighter_levels = fighter_levels;
    out.mage_levels = mage_levels;

    if (copy_to_user(stats, &out, sizeof(out)) > 0) {
        return -EFAULT;
    } else {
        return 0;
    }
}

int sys_rpg_join(pid_t player){
    task_t *player_task = find_task_by_pid(player);
    if (!player_task) {
        return -ESRCH;
    } else if (current->character == NULL || player_task->character == NULL) {
        return -EINVAL;
    }
    list_del(&current->character->list);
    if (list_empty(current->party)) {
        vfree(current->party);
    }
    list_add(&current->character->list, player_task->party);
    return 0;
}
