#include "PQueue.hpp"
#include "CubeDefines.hpp"

//NOTES FOR MYSELF: Use these functions
//  void cube_print(const char* format, ...);
//  void cube_assert_debug(bool condition, const char* file, uint16_t line, const char* str = nullptr, ...);

void test_pqueue_send_receive() {
    PQueue<int, 5> pQueue;
    
    CUBE_PRINT("Testing PQueue Send and Receive\n");

    // Send items to the queue
    for (int i = 0; i < 5; ++i) {
        bool sent = pQueue.Send(i, Priority::NORMAL);
        // Assert that sending was successful
        CUBE_ASSERT(sent, "Sending to PQueue failed");
    }

    // Receive items from the queue
    for (int i = 0; i < 5; ++i) {
        int received;
        bool receivedSuccessfully = pQueue.Receive(received);
        CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
        // Check if Recieved item is correct
        CUBE_ASSERT(received == i, "Received incorrect item from PQueue");
    }
}

void test_pqueue_full() {
    PQueue<int, 2> pQueue;

    CUBE_PRINT("Testing PQueue Full\n");

    // Fill up the queue
    pQueue.Send(1, Priority::NORMAL);
    pQueue.Send(2, Priority::NORMAL);

    // Attempt to send another item
    bool sent = pQueue.Send(3, Priority::NORMAL);
    // Check if enqueuing failed
    CUBE_ASSERT(!sent, "Sending to full PQueue succeeded");
}

void test_pqueue_empty() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Empty\n");

    // Attempt to receive an item from an empty queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    // Assert that receiving fails because the queue is empty
    CUBE_ASSERT(!receivedSuccessfully, "Receiving from empty PQueue succeeded");
}

void test_pqueue_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Priority\n");

    // Send items to the queue with different priorities
    pQueue.Send(1, Priority::LOW);
    pQueue.Send(2, Priority::HIGH);
    pQueue.Send(3, Priority::NORMAL);

    // Receive items from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is the one with the highest priority
    CUBE_ASSERT(received == 2, "Received incorrect item from PQueue");
}

void test_pqueue_receive_wait() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue ReceiveWait\n");

    // Send an item to the queue
    pQueue.Send(1, Priority::NORMAL);

    // Receive an item from the queue using ReceiveWait
    int received;
    bool receivedSuccessfully = pQueue.ReceiveWait(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_current_count() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue GetCurrentCount\n");

    // Send items to the queue
    for (int i = 0; i < 3; ++i) {
        pQueue.Send(i, Priority::NORMAL);
    }

    // Check the current count of the queue
    uint16_t count = pQueue.GetCurrentCount();
    CUBE_ASSERT(count == 3, "Incorrect count in PQueue");
}

void test_pqueue_max_depth() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue GetMaxDepth\n");

    // Check the max depth of the queue
    uint16_t maxDepth = pQueue.GetMaxDepth();
    CUBE_ASSERT(maxDepth == 5, "Incorrect max depth in PQueue");
}

void test_pqueue_send_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Send with High Priority\n");

    // Send an item to the queue with high priority
    bool sent = pQueue.Send(1, Priority::HIGH);
    CUBE_ASSERT(sent, "Sending to PQueue with high priority failed");
}

void test_pqueue_send_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Send with Low Priority\n");

    // Send an item to the queue with low priority
    bool sent = pQueue.Send(1, Priority::LOW);
    CUBE_ASSERT(sent, "Sending to PQueue with low priority failed");
}

void test_pqueue_receive_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Receive with High Priority\n");

    // Send an item to the queue with high priority
    pQueue.Send(1, Priority::HIGH);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_receive_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Receive with Low Priority\n");

    // Send an item to the queue with low priority
    pQueue.Send(1, Priority::LOW);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

int main() {
    test_pqueue_send_receive();
    test_pqueue_full();
    test_pqueue_empty();
    test_pqueue_priority();
    test_pqueue_receive_wait();
    test_pqueue_current_count();
    test_pqueue_max_depth();
    test_pqueue_send_high_priority();
    test_pqueue_send_low_priority();
    test_pqueue_receive_high_priority();
    test_pqueue_receive_low_priority();

    return 0;
}
