#include "PQueue.hpp"
#include "CubeDefines.hpp"
#include "PQueue_Test.hpp"

//NOTES FOR MYSELF: Use these functions
//  void cube_print(const char* format, ...);
//  void cube_assert_debug(bool condition, const char* file, uint16_t line, const char* str = nullptr, ...);

void test_pqueue_send_receive() {
    PQueue<int, 5> testQueue;
    CUBE_PRINT("Size of Queue: %d\n", testQueue.GetMaxDepth());
    CUBE_PRINT("-Testing PQueue Send and Receive-\n");

    //Send items to the queue
    for (int i = 0; i < 5; ++i) {
        bool sent = testQueue.Send(i, Priority::NORMAL);
        osDelay(100);
        // Assert that sending was successful
        if (sent) {
            CUBE_PRINT("|Sending to PQueue succeeded %d\n", sent);
        } else {
            CUBE_PRINT("!Sending to PQueue failed %d\n", sent);
        }
    }
    CUBE_PRINT("-Validating Return Value-\n");


    //Receive items from the queue and checks if data is correct
    for (int i = 0; i < 5; ++i) {
        int received;
        bool receivedSuccessfully = testQueue.Receive(received);
        if (receivedSuccessfully) {
            CUBE_PRINT("|Receiving from PQueue succeeded\n");
        } else {
            CUBE_PRINT("!Receiving from PQueue failed\n");
        }
        if (received == i) {
            CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
        } else {
            CUBE_PRINT("!Received incorrect item from PQueue %d\n", received);
        }
        osDelay(100);
        
    }
    CUBE_PRINT("\n-----End of Test 1 -----\n\n");
}


//Tests if the PQueue adds another item if its full
void test_pqueue_full() {
    PQueue<int, 2> pQueue;

    CUBE_PRINT("-Testing PQueue Full-\n");

    //Fill up the queue
    pQueue.Send(1, Priority::NORMAL);
    pQueue.Send(2, Priority::NORMAL);

    bool sent = pQueue.Send(3, Priority::NORMAL);
    if (sent) {
        CUBE_PRINT("|Sending to full PQueue succeeded\n");
    } else {
        CUBE_PRINT("|Sending to full PQueue failed\n");
    }
    CUBE_PRINT("\n-----End of Test 2 -----\n\n");
}


//Test to receive an item from a empty Queue
void test_pqueue_empty() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Empty-\n");

    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    if( receivedSuccessfully == false){
        CUBE_PRINT( "|Receiving from empty PQueue Success");
    }else{
        CUBE_PRINT( "!Receiving from empty PQueue failed");
    }
    CUBE_PRINT("\n-----End of Test 3 -----\n\n");
}


//Test if first item recieved from queue is highest priority  
void test_pqueue_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Priority-\n");

    pQueue.Send(1, Priority::LOW);
    pQueue.Send(2, Priority::HIGH);
    pQueue.Send(3, Priority::NORMAL);

    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    if (receivedSuccessfully) {
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    } else {
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }
    if(received == 2){
        CUBE_PRINT("|Received correct item from PQueue");
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue");
    }
    CUBE_PRINT("\n-----End of Test 4 -----\n\n");
}

//Test if recieve wait retieves items immediately
void test_pqueue_receive_wait() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue receive awaits-\n"); 
    
    // Send an item to the queue
    pQueue.Send(1, Priority::NORMAL);

    // Receive an item from the queue using ReceiveWait
    int received;
    uint32_t initialTickCount = osKernelSysTick();
    bool receivedSuccessfully = pQueue.ReceiveWait(received);   
    uint32_t finalTickCount = osKernelSysTick();

    //print ticks of the system
    CUBE_PRINT("Initial Tick Count: %d\n", initialTickCount);
    CUBE_PRINT("Final Tick Count: %d\n", finalTickCount);


    if (receivedSuccessfully) {
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    } else {
        CUBE_PRINT("!Receiving from PQueue failed\n");
    } 

    // Check if the time waited is within the expected range
    if (finalTickCount - initialTickCount <= 2) {
        CUBE_PRINT("|ReceiveWait received immediately\n");
    } else {
        CUBE_PRINT("!ReceiveWait did not received immediately\n");
    }
    CUBE_PRINT("\n-----End of Test 5 -----\n\n");
}

//Test if the queue waits for an item to be sent
void test_pqueue_receive_with_timeout() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue receive with timeout-\n"); 

    // Receive an item from the queue using ReceiveWait
    int received;
    uint32_t initialTickCount = osKernelSysTick();
    bool receivedSuccessfully = pQueue.Receive(received, 100);   
    uint32_t finalTickCount = osKernelSysTick();

    // Print the tick count of the system
    CUBE_PRINT("Initial Tick Count: %d\n", initialTickCount);
    CUBE_PRINT("Final Tick Count: %d\n", finalTickCount);

    // Check if the item was received successfully
    if (!receivedSuccessfully) {
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    } else {
        CUBE_PRINT("!Receiving from PQueue failed\n");
    } 

    // Check if the time waited is within the expected range
    if (finalTickCount - initialTickCount == 100) {
        CUBE_PRINT("|ReceiveWait timedout correctly\n");
    } else {
        CUBE_PRINT("!ReceiveWait failed to timeout correctly\n");
    }
    CUBE_PRINT("\n-----End of Test 6 -----\n\n");
}

// Test if the queue returns the correct count of items
void test_pqueue_current_count() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue GetCurrentCount-\n");

    // Send items to the queue
    for (int i = 0; i < 3; ++i) {
        pQueue.Send(i, Priority::NORMAL);
    }

    // Check the current count of the queue
    uint16_t count = pQueue.GetCurrentCount();
    if(count == 3){
        CUBE_PRINT("|Correct count in PQueue: %d\n", count);
    }else{
        CUBE_PRINT("!Incorrect count in PQueue: %d\n", count);
    }
    CUBE_PRINT("\n-----End of Test 7 -----\n\n");
}

// Test if the queue returns the correct max depth
void test_pqueue_max_depth() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue GetMaxDepth-\n");

    // Check the max depth of the queue
    uint16_t maxDepth = pQueue.GetMaxDepth();
    if(maxDepth == 5){
        CUBE_PRINT("|Correct max depth in PQueue: %d\n", maxDepth);
    }else{
        CUBE_PRINT("!Incorrect max depth in PQueue: %d\n", maxDepth);
    }
    CUBE_PRINT("\n-----End of Test 8 -----\n\n");
}

// Test if receive works with 0 timeout
void test_pqueue_receive_zero_timeout() {
    // Setup the PQueue
    PQueue<int, 5> pQueue;
    pQueue.Send(1, Priority::NORMAL);

    CUBE_PRINT("-Testing PQueue Receive with 0 Timeout-\n");

    // Receive an item from the queue using ReceiveWait
    int received;
    bool receivedSuccessfully = pQueue.Receive(received, 0);   

    if (receivedSuccessfully) {
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    } else {
        CUBE_PRINT("!Receiving from PQueue failed\n");
    } 

    if( received == 1){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }
    CUBE_PRINT("\n-----End of Test 9 -----\n\n");
}

// Test if the queue sends high priority items
void test_pqueue_send_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Send with High Priority-\n");

    // Send an item to the queue with high priority
    bool sent = pQueue.Send(1, Priority::HIGH);
    if(sent){
        CUBE_PRINT("|Sending to PQueue with high priority succeeded\n");
    }else{
        CUBE_PRINT("!Sending to PQueue with high priority failed\n");
    }
    CUBE_PRINT("\n-----End of Test 10 -----\n\n");
}

// Test if the queue sends low priority items
void test_pqueue_send_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Send with Low Priority-\n");

    // Send an item to the queue with low priority
    bool sent = pQueue.Send(1, Priority::LOW);
    if(sent){
        CUBE_PRINT("|Sending to PQueue with low priority succeeded\n");
    }else{
        CUBE_PRINT("!Sending to PQueue with low priority failed\n");
    }
    CUBE_PRINT("\n-----End of Test 11 -----\n\n");
}

// Test if the queue receives high priority items
void test_pqueue_receive_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Receive with High Priority-\n");

    // Send an item to the queue with high priority
    pQueue.Send(1, Priority::HIGH);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    if(receivedSuccessfully){
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    }else{
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }

    // Check if received item is correct
    if(received == 1){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }
    CUBE_PRINT("\n-----End of Test 12 -----\n\n");
}

// Test if the queue receives low priority items
void test_pqueue_receive_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Receive with Low Priority-\n");

    // Send an item to the queue with low priority
    pQueue.Send(1, Priority::LOW);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    if(receivedSuccessfully){
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    }else{
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }

    // Check if received item is correct
    if(received == 1){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }
    CUBE_PRINT("\n-----End of Test 13 -----\n\n");
}

// Test if the queue receives items in priority order
void test_pqueue_priority_ordering() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("-Testing PQueue Priority Ordering-\n");

    // Send items to the queue with different priorities
    pQueue.Send(1, Priority::LOW);
    pQueue.Send(2, Priority::HIGH);
    pQueue.Send(3, Priority::NORMAL);

    int received;
    bool receivedSuccessfully;

    // Receive high priority item first
    receivedSuccessfully = pQueue.Receive(received);
    if(receivedSuccessfully){
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    }else{
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }
    if(received == 2){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }
    

    // Receive normal priority item next
    receivedSuccessfully = pQueue.Receive(received);
    if(receivedSuccessfully){
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    }else{
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }
    if(received == 3){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }

    // Receive low priority item last
    receivedSuccessfully = pQueue.Receive(received);
    if(receivedSuccessfully){
        CUBE_PRINT("|Receiving from PQueue succeeded\n");
    }else{
        CUBE_PRINT("!Receiving from PQueue failed\n");
    }
    if(received == 1){
        CUBE_PRINT("|Received correct item from PQueue: %d\n", received);
    }else{
        CUBE_PRINT("!Received incorrect item from PQueue: %d\n", received);
    }
}

// Test if the queue rejects items when the maximum depth is exceeded
void test_pqueue_max_depth_exceeded() {
    PQueue<int, 3> pQueue;

    CUBE_PRINT("-Testing PQueue Maximum Depth Exceeded-\n");

    // Fill up the queue to its maximum depth
    for (int i = 0; i < 3; ++i) {
        bool sent = pQueue.Send(i, Priority::NORMAL);
        CUBE_PRINT(sent, "Sending to PQueue failed");
    }

    // Attempt to send another item, which should fail
    bool sent = pQueue.Send(4, Priority::NORMAL);
    if (!sent) {
        CUBE_PRINT("|Queue rejected item when full\n");
    }else{
        CUBE_PRINT("!Queue Failed and accepted an item\n");
    }

    // Ensure that the queue is still full
    if(pQueue.IsFull()){
        CUBE_PRINT("|PQueue is full\n");
    }else{
        CUBE_PRINT("!PQueue is not full\n");
    }
    CUBE_PRINT("\n-----End of Test 15 -----\n\n");
}

void test_send_to_queue() {
    PQueue<int, 5> pQueue;
}

int main_test() {
    test_pqueue_send_receive();
    test_pqueue_full();
    test_pqueue_empty();
    test_pqueue_priority();
    test_pqueue_receive_wait();
    test_pqueue_receive_with_timeout();
    test_pqueue_current_count();
    test_pqueue_max_depth();
    test_pqueue_receive_zero_timeout();
    test_pqueue_send_high_priority();
    test_pqueue_send_low_priority();
    test_pqueue_receive_high_priority();
    test_pqueue_receive_low_priority();
    test_pqueue_priority_ordering();
    test_pqueue_max_depth_exceeded();
    
    return 0;
}
