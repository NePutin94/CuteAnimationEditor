#include "../include/Tool.h"

void CAE::Tool::SetAsset(std::shared_ptr <AnimationAsset> asset)
{
    this->asset = asset;
    assetUpdated();
}
