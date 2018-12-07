/*
 * elevator look
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

int diskpos = 1; // Disk position is indicated by the most
                 // recently fulfilled request.

struct look_data {
	struct list_head queue;
};

static void look_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
    // elv_dispatch_sort(q, next); // Bump up requests.
}

static int look_dispatch(struct request_queue *q, int force)
{
	struct look_data *nd = q->elevator->elevator_data;

	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
        printk("LOOK: dispatching %llu\n", blk_rq_pos(rq)); // Testing purposes
        diskpos = blk_rq_pos(rq);
		list_del_init(&rq->queuelist); // Delete dispatched request.
		elv_dispatch_sort(q, rq); // Bump up requests.
		return 1;
	}
	return 0;
}
/*
 * Changes are in this function (mostly)!
 *
 * This is for adding elements in queue and making sure it is sorted.
 *
 */

static void look_add_request(struct request_queue *q, struct request *rq)
{
	struct look_data *nd = q->elevator->elevator_data;
    struct list_head * yeet; // List to iterate through
    struct request * item_to_yeet; // items in list



    // If there are other requests in queue ...
    if(!list_empty(&nd->queue)){
        printk("LOOK: adding %llu\n", blk_rq_pos(rq)); // Testing purposes
        // Elevator goes UP from low to high, then jumps back down to low (1-way look)
        list_for_each(yeet, &nd->queue){
           // If we haven't found the position yet for rq to add...
                // Look at request in the yeet and compare it to rq to add
                item_to_yeet = list_entry(yeet, struct request, queuelist);
       
                // If the item index is lower than the disk position 
                if(blk_rq_pos(rq) < diskpos){
                    if(blk_rq_pos(item_to_yeet) < diskpos && blk_rq_pos(rq) < blk_rq_pos(item_to_yeet)){
                            break;
                    }
                }
                else{
                    if(blk_rq_pos(item_to_yeet) < diskpos || blk_rq_pos(rq) < blk_rq_pos(item_to_yeet)){
                            break;
                    }
                }
        }
        //Rq to add is immediately BEFORE item_to_yeet appears
        list_add_tail(&rq->queuelist, yeet); //Add the item, no more checking
    }

    // Queue is empty, just add it LUL
    else{
        printk("LOOK: add empty %llu\n", blk_rq_pos(rq));
	    list_add_tail(&rq->queuelist, &nd->queue);
    }
}

static struct request *
look_former_request(struct request_queue *q, struct request *rq)
{
	struct look_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
look_latter_request(struct request_queue *q, struct request *rq)
{
	struct look_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int look_init_queue(struct request_queue *q, struct elevator_type *e)
{

    printk("I'm initing the queue!\n");
	struct look_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void look_exit_queue(struct elevator_queue *e)
{
	struct look_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_look = {
	.ops = {
		.elevator_merge_req_fn		= look_merged_requests,
		.elevator_dispatch_fn		= look_dispatch,
		.elevator_add_req_fn		= look_add_request,
		.elevator_former_req_fn		= look_former_request,
		.elevator_latter_req_fn		= look_latter_request,
		.elevator_init_fn		= look_init_queue,
		.elevator_exit_fn		= look_exit_queue,
	},
	.elevator_name = "look",
	.elevator_owner = THIS_MODULE,
};

static int __init look_init(void)
{
	return elv_register(&elevator_look);
}

static void __exit look_exit(void)
{
	elv_unregister(&elevator_look);
}

module_init(look_init);
module_exit(look_exit);


MODULE_AUTHOR("Ben Martin & Nickoli Londura, group 7");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Look IO scheduler");
