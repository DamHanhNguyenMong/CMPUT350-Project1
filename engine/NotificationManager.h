#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "GameObject.h"

namespace CMPUT350 {

class NotificationManager {
public:
    void Register(std::weak_ptr<GameObject> object, const std::string& key);
    void Unregister(std::weak_ptr<GameObject> object, const std::string& key);
    void Notify(const std::string& message);
private:
    // Each notification name maps to a list of objects listening for that notification
    std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners; 

};

}  // namespace CMPUT350

#endif