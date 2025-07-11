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

	/*ContainerList.OnInventoryStackChangeDelegate().AddUObject(this,
		&UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);*/
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
	if (Entry.Instance)
	{
		Entry.Instance->OnInventoryStackChangeDelegate().BindUObject(this, &UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);
		Entry.Instance->BroadcastStackChangeMessage(0, Entry.Instance->GetStackCount());
	}
	OnContainerEntryAdded(Entry);// 监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove(const FInvSys_ContainerEntry& Entry)
{
	if (Entry.Instance)
	{
		Entry.Instance->BroadcastStackChangeMessage(Entry.Instance->GetStackCount(), 0);
		Entry.Instance->OnInventoryStackChangeDelegate().Unbind();
	}
	OnContainerEntryRemove(Entry); // 移除监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp,
                                                                  UObject* PreEditPayLoad)
{
	Super::OnConstructInventoryObject(NewInvComp, PreEditPayLoad);
}

UInvSys_EquipSlotWidget* UInvSys_BaseEquipContainerObject::CreateDisplayWidget(APlayerController* PC)
{
	UInvSys_EquipSlotWidget* TempEquipSlotWidget = Super::CreateDisplayWidget(PC);

	return TempEquipSlotWidget;
}

void UInvSys_BaseEquipContainerObject::TryRefreshEquipSlot(const FString& Reason)
{
	Super::TryRefreshEquipSlot(Reason);

	// 根据EquipItemInstance的值判断是移除布局还是添加布局
	/*if (EquipSlotWidget == nullptr)
	{
		return;
	}
	
	if (EquipItemInstance)
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
	else
	{
		ContainerLayout->RemoveFromParent();
		ContainerLayout = nullptr;
		// TODO::删除布局
		// EquipSlotWidget->UnEquipItemInstance();
	}*/
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

bool UInvSys_BaseEquipContainerObject::AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr) return false;
	
	ItemInstance->SetSlotTag(EquipSlotTag);
	ItemInstance->SetInventoryComponent(InventoryComponent);
	
	return ContainerList.AddEntry(ItemInstance);
}

void UInvSys_BaseEquipContainerObject::AddItemInstances(TArray<UInvSys_InventoryItemInstance*> ItemInstances)
{
	for (UInvSys_InventoryItemInstance* ItemInstance : ItemInstances)
	{
		AddItemInstance(ItemInstance);
	}
}

bool UInvSys_BaseEquipContainerObject::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	UE_LOG(LogInventorySystem, Log, TEXT("正在删除容器内的所有物品"))
	bool LOCAL_IsRemoveEquipContainer = Super::RemoveItemInstance(InItemInstance);
	if (LOCAL_IsRemoveEquipContainer)
	{
		//todo::如果是移除的容器本体，那么该怎么处理它内部的子对象呢？将所有的物品打包转移？
		// 如何打包转移这些物品呢？
		// 拖拽容器时，如果是移除的容器本身，那么就只移除容器本身，但是当结束拖拽时，如果是空位置就将库存对象中保存的物品转移至新的背包Actor中
		// 如果是对应Tag的话，同理转移
		
		// Remove All Entries
		
		for (UInvSys_InventoryItemInstance* ContainerEntry : ContainerList.GetAllItems())
		{
			// 将所有物品转移至容器的物品实例中保存。
			InItemInstance->MyInstances.Add(ContainerEntry);
			ContainerList.RemoveEntry(ContainerEntry);
		}
		return true;
	}
	return ContainerList.RemoveEntry(InItemInstance);
}

bool UInvSys_BaseEquipContainerObject::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount)
{
	/*if (ItemInstance == nullptr) return false;
	return ContainerList.UpdateEntryStackCount(ItemInstance, NewStackCount);*/
	return false;
}

void UInvSys_BaseEquipContainerObject::NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	Super::NativeOnEquipItemInstance(InItemInstance);
	if (EquipSlotWidget && InItemInstance)
	{
		auto Fragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		if (Fragment)
		{
			ContainerLayout = CreateWidget<UInvSys_InventoryWidget>(EquipSlotWidget, Fragment->ContainerLayout);
			ContainerLayout->SetInventoryComponent(InventoryComponent);
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

void UInvSys_BaseEquipContainerObject::NativeOnUnEquipItemInstance()
{
	Super::NativeOnUnEquipItemInstance();
	if (EquipSlotWidget && ContainerLayout)
	{
		ContainerLayout->RemoveFromParent();
		ContainerLayout = nullptr;
		// TODO::删除布局
		// EquipSlotWidget->UnEquipItemInstance();
	}
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
	return Super::ContainsItem(UniqueID)/* || ContainerItems.Contains(UniqueID)*/;
}

void UInvSys_BaseEquipContainerObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

}
