#include "GameObject.h"

namespace CMPUT350 {

void GameObject::Initialize(GameContext* context) { return; }
void GameObject::Update(GameContext* context) { return; }
void GameObject::LateUpdate(GameContext* context) { return; }
void GameObject::RenderUI(GameContext* context) { return; }
bool GameObject::IsAlive() const {
    return mAlive;
}
void GameObject::Kill() {
    mAlive = false;
}
void GameObject::ReceiveNotification(const std::string& key) {
    return;
} 
} // namespace CMPUT350
