// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Data/GridInvSys_InventoryItemInstance.h"


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

			FromInvComp->AddItemInstance(ToGridItem, FromGridItem->GetSlotTag(), FromGridItem->GetItemPosition());
			ToInvComp->AddItemInstance(FromGridItem, TempSlotTag, TempItemPosition);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_TryDropItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	// FTimerHandle TempHandler;
	// GetWorld()->GetTimerManager().SetTimer(TempHandler,[this, InItemInstance, InvComp, InPos]()
	// {
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
	// }, 0.2f, false);
}
