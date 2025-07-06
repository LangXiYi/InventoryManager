// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
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
	// 检查位置是否合法
	const FGridInvSys_InventoryItemPosition& ItemPosition = GridContainerItem.ItemPosition;
	FName SlotName = ItemPosition.SlotName;
	if (InventoryObjectMap.Contains(SlotName) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%s 必须在 InventoryObjectMap 中存在的。"), *GridContainerItem.BaseItemData.SlotName.ToString());
		return;
	}
	
	if(InventoryObjectMap[SlotName]->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("SlotName [%s] 不是容器对象。"), *SlotName.ToString());
		return;
	}
	
	// 可装备的网格容器对象
	UGridInvSys_GridEquipContainerObject* ContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InventoryObjectMap[SlotName]);
	if (ContainerObj->IsValidPosition(ItemPosition))
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[%s:%s] Container add item [%s] to [%s]."),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName(),
			*GridContainerItem.BaseItemData.ItemID.ToString(), *SlotName.ToString());
		ContainerObj->AddInventoryItemToContainer(GridContainerItem);
	}
}

void UGridInvSys_InventoryComponent::UpdateContainerItemsPosition(TArray<FName> ChangedItems, TArray<FGridInvSys_InventoryItemPosition> NewItemData)
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
	for (FGridInvSys_InventoryItemPosition ItemData : NewItemData)
	{
		FGridInvSys_InventoryItem TempItem;
		bool bIsFind = FindInventoryItem(ItemData.SlotName, ItemData.Position, TempItem);
		// 判断目标位置的数据是否为待修改的对象，或目标位置为空
		if (bIsFind && ChangedItems.Contains(TempItem.BaseItemData.UniqueID) == false)
		{
			check(false)
			UE_LOG(LogInventorySystem, Error, TEXT("用户可能存在作弊行为，传入数据与服务器数据不匹配。"))
			Client_TryRefreshInventoryObject();
			return;
		}
	}

	// 服务器开始更新数据，执行前请确保数据的正确性。
	for (int i = 0; i < NewItemData.Num(); ++i)
	{
		FGridInvSys_InventoryItem OldItemData;
		FindContainerGridItem(ChangedItems[i], OldItemData);

		UE_LOG(LogInventorySystem, Warning, TEXT("[%s]:%s-%s:(%d,%d) ===> %s-%s:(%d,%d)"),
			*ChangedItems[i].ToString(),
			*OldItemData.ItemPosition.SlotName.ToString(), *OldItemData.ItemPosition.GridID.ToString(),
			OldItemData.ItemPosition.Position.X, OldItemData.ItemPosition.Position.Y,
			*NewItemData[i].SlotName.ToString(), *NewItemData[i].GridID.ToString(),
			NewItemData[i].Position.X, NewItemData[i].Position.Y);
		
		// 判断物品最新的位置与之前的位置在同一容器内
		if (OldItemData.ItemPosition.SlotName == NewItemData[i].SlotName)
		{
			UInvSys_BaseInventoryObject* InventoryObject = InventoryObjectMap[NewItemData[i].SlotName];
			if(InventoryObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
			{
				UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(InventoryObject);
				EquipContainerObj->UpdateInventoryItemFromContainer(ChangedItems[i], NewItemData[i]);
			}
		}
		else
		{
			// 删除旧物品
			UInvSys_BaseInventoryObject* OldInvObject = InventoryObjectMap[OldItemData.ItemPosition.SlotName];
			if(OldInvObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
			{
				UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(OldInvObject);
				EquipContainerObj->RemoveInventoryItemFromContainer(OldItemData);
			}
			// 添加新物品
			UInvSys_BaseInventoryObject* NewInvObject = InventoryObjectMap[NewItemData[i].SlotName];
			if(NewInvObject->IsA(UGridInvSys_GridEquipContainerObject::StaticClass()))
			{
				UGridInvSys_GridEquipContainerObject* EquipContainerObj = Cast<UGridInvSys_GridEquipContainerObject>(NewInvObject);
				FGridInvSys_InventoryItem NewInventoryItemData = OldItemData;
				NewInventoryItemData.ItemPosition = NewItemData[i];
				EquipContainerObj->AddInventoryItemToContainer(NewInventoryItemData);
			}
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

bool UGridInvSys_InventoryComponent::FindEnoughFreeSpace(FName SlotName, FIntPoint ItemSize,
	FGridInvSys_InventoryItemPosition& OutPosition) const
{
	if (InventoryObjectMap.Contains(SlotName))
	{
		UInvSys_BaseInventoryObject* InvObj = InventoryObjectMap[SlotName];
		if (InvObj->IsA(UInvSys_BaseEquipContainerObject::StaticClass()))
		{
			OutPosition.SlotName = SlotName;
			OutPosition.ItemSize = ItemSize;
			// OutPosition.Direction = EGridInvSys_ItemDirection::Horizontal;
			UGridInvSys_GridEquipContainerObject* ContainerObject = Cast<UGridInvSys_GridEquipContainerObject>(InvObj);
			return ContainerObject->FindEnoughFreeSpace(ItemSize, OutPosition);
		}
	}
	return false;
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

void UGridInvSys_InventoryComponent::GetAllContainerSlotName(TArray<FName>& OutArray) const
{
	for (auto InventoryObjectTuple : InventoryObjectMap)
	{
		if (InventoryObjectTuple.Value->IsA(UInvSys_BaseEquipContainerObject::StaticClass()))
		{
			OutArray.Add(InventoryObjectTuple.Key);
		}
	}
}

void UGridInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}
