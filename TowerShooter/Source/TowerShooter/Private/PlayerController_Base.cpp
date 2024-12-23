// ©David John Nolan, 2024


#include "PlayerController_Base.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/NavigationConfig.h"


void APlayerController_Base::BeginPlay()
{
    Super::BeginPlay();

    FSlateApplication::Get().GetNavigationConfig().Get().KeyEventRules.Emplace(EKeys::A, EUINavigation::Left);
    FSlateApplication::Get().GetNavigationConfig().Get().KeyEventRules.Emplace(EKeys::S, EUINavigation::Down);
    FSlateApplication::Get().GetNavigationConfig().Get().KeyEventRules.Emplace(EKeys::W, EUINavigation::Up);
    FSlateApplication::Get().GetNavigationConfig().Get().KeyEventRules.Emplace(EKeys::D, EUINavigation::Right);


}
