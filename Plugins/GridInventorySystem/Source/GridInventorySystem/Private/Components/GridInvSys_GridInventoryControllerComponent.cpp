// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Data/GridInvSys_InventoryItemInstance.h"


UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
}

void UGridInvSys_GridInventoryControllerComponent::Server_UpdateItemInstancePosition_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* ItemInstance,
	FGridInvSys_ItemPosition NewPosition)
{
	check(ItemInstance)
	if (ItemInstance)
	{
		UGridInvSys_InventoryItemInstance* GridItem = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		UInvSys_InventoryComponent* OldInvComp = ItemInstance->GetInventoryComponent();
		check(OldInvComp && InvComp && GridItem)
		if (OldInvComp && InvComp && GridItem)
		{
			OldInvComp->RemoveItemInstance(ItemInstance); // 将物品从原先的库存组件中删除
			InvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(GridItem, NewPosition.EquipSlotTag, NewPosition);
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
			UE_LOG(LogInventorySystem, Log, TEXT("交换两物品的位置 {%s:%s} <----> {%s%s}"),
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
	bool bIsSuccess = false;
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
	if (GridItemInstance)
	{
		bIsSuccess = TryDropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, GridItemInstance,
			InPos.EquipSlotTag, InPos);
	}
	UE_LOG(LogInventorySystem, Log, TEXT("[Server:%s] 尝试放置物品[%s] --> {%s}"),
		bIsSuccess ? TEXT("TRUE") : TEXT("FALSE"),
		*InItemInstance->GetItemDisplayName().ToString(), *InPos.ToString())
}
