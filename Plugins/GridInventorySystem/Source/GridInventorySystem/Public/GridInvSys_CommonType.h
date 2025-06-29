
#pragma once

#include "InvSys_CommonType.h"

#include "GridInvSys_CommonType.generated.h"

UENUM(BlueprintType)
enum class EGridInvSys_ItemDirection : uint8
{
	Horizontal = 0,
	Vertical
};

UENUM(BlueprintType)
enum class EGridInvSys_InventoryItemType : uint8
{
	None = 0,
	// 主武器
	Weapon_Primary,
	// 副武器
	Weapon_Secondary,
	// 近战武器
	Weapon_Melee,

	// 头甲
	Armor_Helmet,
	// 胸甲
	Armor_Breastplate,

	// 胸挂
	Container_ChestRig,
	// 口袋
	Container_Pocket,
	// 背包
	Container_Backpack,
	// 安全箱
	Container_SafeBox,

	// 子弹
	Item_Default,
};

USTRUCT(BlueprintType)
struct FGridInvSys_InventoryItemPosition
{
	GENERATED_BODY()

	// 网格ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName GridID;
	// 单元格位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FIntPoint Position = FIntPoint(-1, -1);
	// 物品方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	EGridInvSys_ItemDirection Direction = EGridInvSys_ItemDirection::Horizontal;
	// 物品大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FIntPoint Size = FIntPoint(1, 1);
};

USTRUCT(BlueprintType)
struct FGridInvSys_InventoryItem
{
	GENERATED_BODY()

	// 物品类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	EGridInvSys_InventoryItemType ItemType = EGridInvSys_InventoryItemType::None;
	// 物品基础数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FInvSys_InventoryItem BaseItemData;
	// 物品在网格中的位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FGridInvSys_InventoryItemPosition ItemPosition;

	bool operator==(const FGridInvSys_InventoryItem& Item) const
	{
		return BaseItemData.UniqueID == Item.BaseItemData.UniqueID;
	}
};