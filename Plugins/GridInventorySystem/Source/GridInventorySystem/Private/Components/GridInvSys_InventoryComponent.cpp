// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryObject/GridInvSys_GridContainerObject.h"
#include "Components/InventoryObject/GridInvSys_GridEquipContainerObject.h"


// Sets default values for this component's properties
UGridInvSys_InventoryComponent::UGridInvSys_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

void UGridInvSys_InventoryComponent::AddInventoryItemToGridContainer(FGridInvSys_InventoryItem GridContainerItem)
{
	FName SlotName = GridContainerItem.BaseItemData.SlotName;
	if (InventoryObjectMap.Contains(SlotName) == false)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("%s 必须在 InventoryObjectMap 中存在的。"), *GridContainerItem.BaseItemData.SlotName.ToString());
		return;
	}
	if (InventoryObjectMap[SlotName]->IsA(UGridInvSys_GridContainerObject::StaticClass()))
	{
		// 网格容器对象
		if (UGridInvSys_GridContainerObject* ContainerObj = Cast<UGridInvSys_GridContainerObject>(InventoryObjectMap[SlotName]))
		{
			ContainerObj->AddInventoryItemToContainer(GridContainerItem);
		}
	}
	else if(InventoryObjectMap[SlotName]->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
	{
		// 可装备的网格容器对象
		if (UGridInvSys_GridEquipContainerObject* ContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InventoryObjectMap[SlotName]))
		{
			ContainerObj->AddInventoryItemToContainer(GridContainerItem);
		}
	}
}

void UGridInvSys_InventoryComponent::UpdateContainerItemsPosition(TArray<FName> ChangedItems, TArray<FGridInvSys_InventoryItem> NewItemData)
{
	if (HasAuthority() == false)
	{
		return;
	}

	// 验证数据在服务器中是否存在
	for (FName ChangedItemUniqueID : ChangedItems)
	{
		if (IsContainsInventoryItem(ChangedItemUniqueID) == false)
		{
			check(false)
			UE_LOG(LogInventorySystem, Error, TEXT("用户可能存在作弊行为，传入数据与服务器数据不匹配。"))
			Client_TryRefreshInventoryObject();
			return;
		}
	}

	// 验证目标位置的数据在已修改列表中
	for (FGridInvSys_InventoryItem ItemData : NewItemData)
	{
		FGridInvSys_InventoryItem TempItem;
		bool bIsFind = FindInventoryItem(ItemData.BaseItemData.SlotName, ItemData.ItemPosition.Position, TempItem);
		// 判断目标位置的数据是否为待修改的对象，或目标位置为空
		if (bIsFind && ChangedItems.Contains(TempItem.BaseItemData.UniqueID) == false)
		{
			check(false)
			UE_LOG(LogInventorySystem, Error, TEXT("用户可能存在作弊行为，传入数据与服务器数据不匹配。"))
			Client_TryRefreshInventoryObject();
			return;
		}
	}

	for (FGridInvSys_InventoryItem ItemData : NewItemData)
	{
		FGridInvSys_InventoryItem OldItemData;
		bool bIsFind = FindContainerGridItem(ItemData.BaseItemData.UniqueID, OldItemData);
		if (bIsFind && InventoryObjectMap.Contains(ItemData.BaseItemData.SlotName))
		{
			UInvSys_BaseInventoryObject* InventoryObject = InventoryObjectMap[ItemData.BaseItemData.SlotName];
			if (InventoryObject->IsA(UGridInvSys_GridContainerObject::StaticClass()))
			{
				UGridInvSys_GridContainerObject* ContainerObj = Cast<UGridInvSys_GridContainerObject>(InventoryObject);
				ContainerObj->UpdateInventoryItemFromContainer(ItemData);
			}
			if(InventoryObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
			{
				UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InventoryObject);
				EquipContainerObj->UpdateInventoryItemFromContainer(ItemData.BaseItemData.UniqueID, ItemData.ItemPosition);
			}
		}
		else
		{
			check(false)
			UE_LOG(LogInventorySystem, Error, TEXT("用户可能存在作弊行为，传入数据与服务器数据不匹配。"))
			Client_TryRefreshInventoryObject();
			return;
		}
	}
}

void UGridInvSys_InventoryComponent::UpdateOtherContainerItemsPosition(UGridInvSys_InventoryComponent* ToInvCom,
	TArray<FGridInvSys_InventoryItem> FromItemData, TArray<FGridInvSys_InventoryItem> ToItemData)
{
	/*
	 * 检查AddDataFrom在From组件中是否存在
	 * 检查AddDataTo在To组件中是否存在
	 * 检查AddDataFrom在To组件中的合法性
	 * 检查AddDataTo在From组件中的合法性
	 * 验证通过，向FromInvCom发送删除命令以及添加命令
	 * 验证通过，向ToInvCom发送删除命令以及添加命令
	 */
	// todo::发送的删除命令与添加命令若与客户端冲突如何处理？直接整体刷新？
}

// Called when the game starts
void UGridInvSys_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGridInvSys_InventoryComponent::NativeOnInitInventoryObjects(APlayerController* InController)
{
	Super::NativeOnInitInventoryObjects(InController);
	check(InController)
	InventoryLayoutWidget = CreateWidget(InController, InventoryLayoutWidgetClass);
}

bool UGridInvSys_InventoryComponent::FindInventoryItem(FName SlotName, const FIntPoint& ItemPosition, FGridInvSys_InventoryItem& OutItem)
{
	if (InventoryObjectMap.Contains(SlotName))
	{
		UInvSys_BaseInventoryObject* InventoryObject = InventoryObjectMap[SlotName];
		if (InventoryObject->IsA(UGridInvSys_GridContainerObject::StaticClass()))
		{
			const UGridInvSys_GridContainerObject* ContainerObj = Cast<UGridInvSys_GridContainerObject>(InventoryObject);
			return ContainerObj->FindContainerGridItem(ItemPosition, OutItem);

		}
		if(InventoryObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
		{
			const UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InventoryObject);
			return EquipContainerObj->FindContainerGridItem(ItemPosition, OutItem);
		}
	}
	return false;
}

bool UGridInvSys_InventoryComponent::FindContainerGridItem(FName ItemUniqueID,  FGridInvSys_InventoryItem& OutItem)
{
	for (UInvSys_BaseInventoryObject* InvObject : InventoryObjectList)
	{
		if (InvObject->IsA(UGridInvSys_GridContainerObject::StaticClass()))
		{
			const UGridInvSys_GridContainerObject* ContainerObj = Cast<UGridInvSys_GridContainerObject>(InvObject);
			if (ContainerObj->FindContainerGridItem(ItemUniqueID, OutItem))
			{
				return true;
			}
		}
		if(InvObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
		{
			const UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InvObject);
			if (EquipContainerObj->FindContainerGridItem(ItemUniqueID, OutItem))
			{
				return true;
			}
		}
	}
	return false;
}

void UGridInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}
