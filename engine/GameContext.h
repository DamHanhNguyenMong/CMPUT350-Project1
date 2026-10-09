#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H

#include "DrawContext.h"
#include "EngineView.h"
#include "GameObject.h"
# include "NotificationManager.h"

namespace CMPUT350 {

class GameContext {
public:
    EngineView* mEngineView; // lets an object interact with the engine
    DrawContext* ScreenContext; // draws gameplay objects with their position and rotation transforms
    DrawContext* GUIContext; // draws UI elements without gameplay transforms
    std::weak_ptr<GameObject> CurrObject; // a weak pointer to the current object, useful when creating 
                                        //  references to objects without keeping them alive
    NotificationManager* mNotificationManager; // lets objects work with the notification system
    int currentFrame; // provides a shared game-frame counter
};

}  // namespace CMPUT350

#endif  // GAMECONTEXT_H
