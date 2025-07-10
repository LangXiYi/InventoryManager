// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"

#include "BaseInventorySystem.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"
#include "Blueprint/UserWidget.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipContainerSlotWidget.h"
#include "Widgets/InvSys_EquipSlotWidget.h"


UInvSys_BaseEquipContainerObject::UInvSys_BaseEquipContainerObject()
	:ContainerList(this)
{
	ContainerList.OnContainerEntryAddedDelegate().AddUObject(this,
		&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded);
	
	ContainerList.OnContainerEntryRemoveDelegate().AddUObject(this,
		&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove);

	ContainerList.OnInventoryStackChangeDelegate().AddUObject(this,
		&UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);
}

bool UInvSys_BaseEquipContainerObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                           FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FInvSys_ContainerEntry Entry : ContainerList.Entries)
	{
		UInvSys_InventoryItemInstance* Instance = Entry.Instance;
		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
		// UE_LOG(LogInventorySystem, Error, TEXT("复制 Fast Array 的子对象 = %s]"), *Instance->GetName());
	}
	return WroteSomething;
}

void UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange(FInvSys_InventoryStackChangeMessage ChangeInfo)
{
	OnInventoryStackChange(ChangeInfo); // 监听 Stack Count 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryAdded(Entry);// 监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryRemove(Entry); // 移除监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp,
                                                                  UObject* PreEditPayLoad)
{
	Super::OnConstructInventoryObject(NewInvComp, PreEditPayLoad);
}

UInvSys_EquipSlotWidget* UInvSys_BaseEquipContainerObject::CreateEquipSlotWidget(APlayerController* PC)
{
	UInvSys_EquipSlotWidget* TempEquipSlotWidget = Super::CreateEquipSlotWidget(PC);

	return TempEquipSlotWidget;
}

void UInvSys_BaseEquipContainerObject::TryRefreshEquipSlot(const FString& Reason)
{
	Super::TryRefreshEquipSlot(Reason);
	if (EquipItemInstance && EquipSlotWidget) // 刷新物品时同步刷新容器控件
	{
		auto Fragment = EquipItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		if (Fragment)
		{
			ContainerLayout = CreateWidget<UInvSys_InventoryWidget>(EquipSlotWidget, Fragment->ContainerLayout);
			if (EquipSlotWidget->IsA(UInvSys_EquipContainerSlotWidget::StaticClass()))
			{
				UInvSys_EquipContainerSlotWidget* TempContainerSlotWidget = Cast<UInvSys_EquipContainerSlotWidget>(EquipSlotWidget);
				check(TempContainerSlotWidget);
				TempContainerSlotWidget->AddContainerLayout(ContainerLayout);
				TryRefreshContainerItems();
			}
		}
	}
}

void UInvSys_BaseEquipContainerObject::TryRefreshContainerItems()
{
	UE_LOG(LogInventorySystem, Log, TEXT("正在刷新容器内所有物品！"))
	for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	{
		ContainerList.BroadcastRemoveEntryMessage(Entry);
	}
	for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	{
		ContainerList.BroadcastAddEntryMessage(Entry);
	}
}

bool UInvSys_BaseEquipContainerObject::AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance, int32 StackCount)
{
	if (ItemInstance == nullptr) return false;
	return ContainerList.AddEntry(ItemInstance, StackCount);
}

bool UInvSys_BaseEquipContainerObject::RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance != nullptr) return false;
	return ContainerList.RemoveEntry(ItemInstance);
}

bool UInvSys_BaseEquipContainerObject::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount)
{
	if (ItemInstance == nullptr) return false;
	return ContainerList.UpdateEntryStackCount(ItemInstance, NewStackCount);
}

void UInvSys_BaseEquipContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseEquipContainerObject, ContainerList);
}

const UInvSys_InventoryItemInstance* UInvSys_BaseEquipContainerObject::FindItemInstance(FGuid ItemUniqueID) const
{
	if (ContainerEntryMap.Contains(ItemUniqueID))
	{
		return ContainerEntryMap[ItemUniqueID].Instance;
	}
	return nullptr;
}

bool UInvSys_BaseEquipContainerObject::ContainsItem(FName UniqueID)
{
	return Super::ContainsItem(UniqueID) || ContainerItems.Contains(UniqueID);
}

void UInvSys_BaseEquipContainerObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

}
