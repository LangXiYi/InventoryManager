// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"


UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
}

void UGridInvSys_GridInventoryControllerComponent::TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos)
{
	bool bIsSuccess = false;
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
	const FGridInvSys_ItemPosition& OldPosition = GridItemInstance->GetItemPosition();
	if (GridItemInstance)
	{
		bIsSuccess = DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, GridItemInstance,
			InPos.EquipSlotTag, InPos);
	}
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[Server:%s] 更新物品位置信息[%s] {%s}--> {%s}"),
		bIsSuccess ? TEXT("TRUE") : TEXT("FALSE"),
		*InItemInstance->GetItemDisplayName().ToString(),
		*OldPosition.ToString(), *InPos.ToString())
}

void UGridInvSys_GridInventoryControllerComponent::Server_TestFunc_Implementation(UInvSys_InventoryComponent* InvComp,
	const TArray<UGridInvSys_InventoryItemInstance*>& Array, const TArray<FGridInvSys_ItemPosition>& NewItemPositions,
	UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition)
{
	UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);

	int32 Num = Array.Num();
	for (int i = 0; i < Num; ++i)
	{
		GridInvComp->UpdateItemInstancePosition(Array[i], NewItemPositions[i]);
	}
	DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, ItemInstance, DropPosition.EquipSlotTag, DropPosition);
}

void UGridInvSys_GridInventoryControllerComponent::Server_AddItemInstancesToContainerPos_Implementation(
	UInvSys_InventoryComponent* InvComp, const TArray<UInvSys_InventoryItemInstance*>& InItemInstances,
	const TArray<FGridInvSys_ItemPosition>& InPosArray)
{
	check(InvComp)
	if (InvComp && InvComp->IsA<UGridInvSys_InventoryComponent>())
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
		int32 ItemNum = InItemInstances.Num();
		int32 PosNum = InPosArray.Num();

		check(GridInvComp)
		check(ItemNum == PosNum)
		if (GridInvComp && ItemNum == PosNum)
		{
			for (int i = 0; i < ItemNum; ++i)
			{
				GridInvComp->AddItemInstanceToContainerPos(InItemInstances[i], InPosArray[i]);
			}
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_RestoreItemInstanceToPos_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	check(InvComp)
	if (InvComp && InvComp->IsA<UGridInvSys_InventoryComponent>())
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
		GridInvComp->RestoreItemInstanceToPos(InItemInstance, InPos);
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_UpdateItemInstancePosition_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* ItemInstance,
	FGridInvSys_ItemPosition NewPosition)
{
	check(ItemInstance)
	if (ItemInstance)
	{
		// UGridInvSys_InventoryItemInstance* GridItem = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		UGridInvSys_InventoryComponent* GridComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
		check(GridComp)
		if (GridComp)
		{
			GridComp->UpdateItemInstancePosition(ItemInstance, NewPosition);
			// OldInvComp->RemoveItemInstance(ItemInstance); // 将物品从原先的库存组件中删除
			// InvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(GridItem, NewPosition.EquipSlotTag, NewPosition);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_SwapItemInstance_Implementation(
	UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance)
{
	UGridInvSys_InventoryItemInstance* FromGridItem = Cast<UGridInvSys_InventoryItemInstance>(FromItemInstance);
	UGridInvSys_InventoryItemInstance* ToGridItem = Cast<UGridInvSys_InventoryItemInstance>(ToItemInstance);
	check(FromGridItem)
	check(ToGridItem)
	if (FromGridItem && ToGridItem)
	{
		UInvSys_InventoryComponent* FromInvComp = FromGridItem->GetInventoryComponent<UInvSys_InventoryComponent>();
		UInvSys_InventoryComponent* ToInvComp = ToGridItem->GetInventoryComponent<UInvSys_InventoryComponent>();
		if (FromInvComp != nullptr && ToInvComp != nullptr)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("交换两物品的位置 {%s:%s} <----> {%s%s}"),
				*FromItemInstance->GetItemDisplayName().ToString(), *FromGridItem->GetItemPosition().ToString(),
				*ToGridItem->GetItemDisplayName().ToString(), *ToGridItem->GetItemPosition().ToString())

			// 先将两个物品从库存中移除
			FromInvComp->RemoveItemInstance(FromGridItem);
			ToInvComp->RemoveItemInstance(ToGridItem);
			FromInvComp->GetOwner()->ForceNetUpdate();
			if (FromInvComp != ToInvComp)
			{
				ToInvComp->GetOwner()->ForceNetUpdate();
			}

			// 这里需要提前缓存ToItem的位置信息，避免在第一次添加执行完成后修改FromItem到To位置时的数据被污染。
			FGameplayTag TempSlotTag = ToGridItem->GetSlotTag();
			FGridInvSys_ItemPosition TempItemPosition = ToGridItem->GetItemPosition();

			FromInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(ToGridItem, FromGridItem->GetSlotTag(), FromGridItem->GetItemPosition());
			ToInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(FromGridItem, TempSlotTag, TempItemPosition);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_TryDropItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	if (InvComp == nullptr || InItemInstance == nullptr)
	{
		checkf(false, TEXT("传入的库存组件或物品实例为空"));
		return;
	}
	bool bIsSuccess = false;
	auto ContainerFragment = InvComp->FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(InPos.EquipSlotTag);
	if (InItemInstance)
	{
		FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(InItemInstance, InPos.Direction);
		bool bHasEnoughFreeSpace = ContainerFragment->HasEnoughFreeSpace(InPos.Position, InPos.GridID, ItemSize);

		UE_CLOG(bHasEnoughFreeSpace == false, LogInventorySystem, Error, TEXT("位置 %s 无法容纳大小为 %s 的 %s"),
			*InPos.ToString(), *ItemSize.ToString(), *InItemInstance->GetItemDisplayName().ToString())

		if (bHasEnoughFreeSpace)
		{
			bIsSuccess = DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, InItemInstance, InPos.EquipSlotTag, InPos);
		}
	}
	UE_CLOG(bIsSuccess == false, LogInventorySystem, Error, TEXT("放置 %s 失败 ---> %s"),
		*InItemInstance->GetItemDisplayName().ToString(), *InPos.ToString())
}
