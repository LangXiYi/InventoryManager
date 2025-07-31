
#pragma once

#include "GameplayTagContainer.h"
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

	// 槽位名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName SlotName = NAME_None;
	// 网格ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FName GridID = NAME_None;
	// 单元格位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FIntPoint Position = FIntPoint(0, 0);
	// 物品方向会影响贴图方向以及占据的网格大小。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	EGridInvSys_ItemDirection Direction = EGridInvSys_ItemDirection::Horizontal;
	// 物品在网格占据的大小，受方向影响，如1，2的物品在旋转后应该为2，1，但物品的默认大小为1，2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FIntPoint ItemSize = FIntPoint(1, 1);
};

USTRUCT(BlueprintType)
struct FGridInvSys_ItemPosition
{
	GENERATED_BODY()
	// 容器名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FGameplayTag EquipSlotTag = FGameplayTag();
	// 网格ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	int32 GridID = -1;
	// 单元格位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FIntPoint Position = FIntPoint(0, 0);
	// 物品方向会影响贴图方向以及占据的网格大小。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	EGridInvSys_ItemDirection Direction = EGridInvSys_ItemDirection::Horizontal;

	FString ToString() const
	{
		FString StrDirection = "None";
		switch (Direction)
		{
		case EGridInvSys_ItemDirection::Horizontal:
			StrDirection = "Horizontal";
			break;
		case EGridInvSys_ItemDirection::Vertical:
			StrDirection = "Vertical";
			break;
		default: ;
		}
		return EquipSlotTag.ToString() + "_" + FString::FromInt(GridID) +
			":[" + FString::FromInt(Position.X) + ", " + FString::FromInt(Position.Y) + ":" + StrDirection + "]";
	}

	bool operator==(const FGridInvSys_ItemPosition& Right) const
	{
		return (EquipSlotTag == Right.EquipSlotTag)
			&& (GridID == Right.GridID)
			&& (Position == Right.Position)
			&& (Direction == Right.Direction);
	}

	bool operator!=(const FGridInvSys_ItemPosition& Right) const
	{
		return (EquipSlotTag != Right.EquipSlotTag)
			|| (GridID != Right.GridID)
			|| (Position != Right.Position)
			|| (Direction != Right.Direction);
	}
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
		return BaseItemData == Item.BaseItemData;
	}
};