
#pragma once

#include "InvSys_CommonType.generated.h"


class UInvSys_InventoryItemInfo;

UENUM()
enum class EInvSys_ItemQuality : uint8
{
	None = 0,
	NO_0,
	NO_1,
	NO_2,
	NO_3,
	NO_4,
	NO_5,
};

USTRUCT(BlueprintType)
struct FInvSys_InventoryItem
{
	GENERATED_BODY()
	// 槽名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName SlotName;
	// 物品的唯一ID，在不考虑差异性的前提下可以与 物品ID 一致。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName UniqueID;
	// 物品ID，用以区分物品种类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName ItemID;
	// 物品是否被装备
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	bool bIsEquipped = false;
	// 物品是否可以堆叠存放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	bool bIsStack = false;
	// 物品数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	int32 Count = 1;
	// 物品信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	TObjectPtr<UInvSys_InventoryItemInfo> ItemInfo = nullptr;

	// 物品信息的AssetID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FPrimaryAssetId ItemInfoAssetID;
	
};
