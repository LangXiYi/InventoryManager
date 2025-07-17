// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Misc/LowLevelTestAdapter.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipSlotWidget.h"

UInvSys_BaseEquipmentObject::UInvSys_BaseEquipmentObject()
{
}

void UInvSys_BaseEquipmentObject::AddInventoryItemToEquipSlot_DEPRECATED(const FInvSys_InventoryItem& NewItem)
{
}

UInvSys_InventoryItemInstance* UInvSys_BaseEquipmentObject::EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef)
{
	UInvSys_InventoryItemInstance* TempItemInstance = NewObject<UInvSys_InventoryItemInstance>(GetOwner());
	if (TempItemInstance)
	{
		TempItemInstance->SetItemDefinition(ItemDef);
		TempItemInstance->SetInventoryComponent(InventoryComponent);
		TempItemInstance->SetItemUniqueID(FGuid::NewGuid());
		TempItemInstance->SetSlotTag(EquipSlotTag);
		if (TempItemInstance)
		{
			EquipInventoryItem(TempItemInstance);
		}
	}
	return TempItemInstance;
}

void UInvSys_BaseEquipmentObject::EquipInventoryItem(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (HasAuthority())
	{
		EquipItem = NewItemInstance;
		EquipItem->SetSlotTag(EquipSlotTag);
		EquipItem->SetInventoryComponent(InventoryComponent);
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_EquipItemInstance();
		}
	}
}

void UInvSys_BaseEquipmentObject::UnEquipInventoryItem()
{
	if (HasAuthority())
	{
		// EquipItemInstance->RemoveFromInventory();
		EquipItem = nullptr;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_EquipItemInstance();
		}
	}
}

bool UInvSys_BaseEquipmentObject::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{

	// Super::RemoveItemInstance(InItemInstance);
	if (InItemInstance == EquipItem)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("正在删除装备槽中的物品"))
		UnEquipInventoryItem();
		return true;
	}
	return false;
}

bool UInvSys_BaseEquipmentObject::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	if (InItemInstance && EquipItem == nullptr)
	{
		EquipInventoryItem(InItemInstance);
		return true;
	}
	return false;
}

UInvSys_EquipSlotWidget* UInvSys_BaseEquipmentObject::CreateDisplayWidget(APlayerController* PC)
{
	if (PC != nullptr && PC->IsLocalController())
	{
		EquipSlotWidget = CreateWidget<UInvSys_EquipSlotWidget>(PC, EquipSlotWidgetClass);
		EquipSlotWidget->SetInventoryObject(this);

		OnRep_EquipItemInstance();
		//TryRefreshEquipSlot();
	}
	return EquipSlotWidget;
}

void UInvSys_BaseEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseEquipmentObject, EquipItem);
}

void UInvSys_BaseEquipmentObject::TryRefreshEquipSlot(const FString& Reason)
{
	// todo::重构刷新逻辑
	/*if (EquipSlotWidget)
	{
		if (EquipItemInstance)
		{
			EquipSlotWidget->EquipItemInstance(EquipItemInstance);
		}
		else
		{
			EquipSlotWidget->UnEquipItemInstance();
		}
	}*/
}

void UInvSys_BaseEquipmentObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

	COPY_INVENTORY_OBJECT_PROPERTY(UInvSys_PreEditEquipmentObject, EquipSlotWidgetClass);
}

void UInvSys_BaseEquipmentObject::NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(EquipSlotWidget);
	check(EquipItem);
	EquipSlotWidget->EquipItemInstance(InItemInstance);
}

void UInvSys_BaseEquipmentObject::NativeOnUnEquipItemInstance()
{
	check(EquipSlotWidget);
	EquipSlotWidget->UnEquipItemInstance();
}

bool UInvSys_BaseEquipmentObject::ContainsItem(FGuid ItemUniqueID)
{
	//check(EquipItem)
	return EquipItem ? EquipItem->GetItemUniqueID() == ItemUniqueID : false;
	//return GetOccupantData().UniqueID == UniqueID;
}

bool UInvSys_BaseEquipmentObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (EquipItem && IsValid(EquipItem))
	{
		WroteSomething |= Channel->ReplicateSubobject(EquipItem, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void UInvSys_BaseEquipmentObject::OnRep_EquipItemInstance()
{
	if (EquipSlotWidget)
	{
		if (EquipItem)
		{
			NativeOnEquipItemInstance(EquipItem);
		}
		else if(LastEquipItemInstance.IsValid())
		{
			NativeOnUnEquipItemInstance();
		}
	}
	LastEquipItemInstance = EquipItem;
}
