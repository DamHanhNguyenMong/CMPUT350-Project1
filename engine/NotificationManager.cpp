#include "NotificationManager.h"

#include <algorithm>

namespace CMPUT350 {

// adds an object to the listener list for a notification, avoiding duplicate registrations
void NotificationManager::Register(std::weak_ptr<GameObject> object, const std::string& key) {
    // Only register if the object is still alive
    if (object.expired()) {
        return;
    }
    // Get the list of listeners for the given key, or create a new list if it doesn't exist
    auto& listeners = mListeners[key];
    // Avoid registering the same object twice
    for (const auto& listener : listeners) {
        if (!listener.owner_before(object) && !object.owner_before(listener)) {
            return;
        }
    }
    // Add the object to the list of listeners for the given key
    listeners.push_back(object);
}
// Removes that object from the specified notification's list
void NotificationManager::Unregister(std::weak_ptr<GameObject> object, const std::string& key) {
    // Get the list of listeners for the given key
    auto it = mListeners.find(key);
    // If the key doesn't exist, there's nothing to unregister
    if (it == mListeners.end()) {
        return;
    }
    auto& listeners = it->second; // second is the vector of listeners
    // Remove the object from the list of listeners for the given key
    listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
        [&object](const std::weak_ptr<GameObject>& listener) {
            // Remove the listener if it matches the object or if it has expired
            return listener.expired() || (!listener.owner_before(object) && !object.owner_before(listener));
        }), listeners.end());
    // If the list of listeners is now empty, remove the key from the map
    if (listeners.empty()) {
        mListeners.erase(it);
    }
}
// calls ReceiveNotification() on each living listener and removes expired weak pointers.
void NotificationManager::Notify(const std::string& message) {
    // Find the mListeners for the given message
    auto it = mListeners.find(message);
    // If there are no listeners for this message, return
    if (it == mListeners.end()) {
        return;
    }
    // Copy the list so callbacks can safely change registrations
    auto listeners = it->second;
    // Notify each listener
    for (const auto& listener : listeners) {
        if (auto object = listener.lock()) {
            object->ReceiveNotification(message);
        }
    }
    // Remove expired objects from the original listener list
    it = mListeners.find(message);
    // If the key still exists, clean up expired listeners
    if (it != mListeners.end()) {
        auto& originalListeners = it->second;
        originalListeners.erase(std::remove_if(originalListeners.begin(), originalListeners.end(),
            [](const std::weak_ptr<GameObject>& listener) {
                // Remove the listener if it has expired
                return listener.expired();
            }), originalListeners.end());
        // If the list of listeners is now empty, remove the key from the map
        if (originalListeners.empty()) {
            mListeners.erase(it);
        }
    }
    // Notify() only delivers notifications to objects registered under the exact same string
}
}  // namespace CMPUT350