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
	if (ContainerList.OnContainerEntryAddedDelegate().IsBound() == false)
	{
		ContainerList.OnContainerEntryAddedDelegate().BindUObject(this,
			&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded);
	}

	if (ContainerList.OnContainerEntryRemoveDelegate().IsBound() == false)
	{
		ContainerList.OnContainerEntryRemoveDelegate().BindUObject(this,
			&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove);
	}
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

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsForceRep)
{
	if (Entry.Instance)
	{
		Entry.Instance->OnInventoryStackChangeDelegate().BindUObject(this, &UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);
		// Client：!!!客户端环境下复制的对象值优于该对象的属性到达客户端，即先执行该函数然后再执行对象的OnRep函数。
		// Server: !!!服务器环境则于客户端相反，对象属性的OnRep函数先执行，然后再执行该函数。
		// 所以为了实现正确的效果，同时避免客户端重复广播改变事件，所以限制条件为初始化阶段或服务器环境下才广播
		if (bIsForceRep || HasAuthority())
		{
			Entry.Instance->BroadcastStackChangeMessage(0, Entry.Instance->GetStackCount());
		}
	}
	OnContainerEntryAdded(Entry, bIsForceRep);// 监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsForceRep)
{
	if (Entry.Instance)
	{
		Entry.Instance->BroadcastStackChangeMessage(Entry.Instance->GetStackCount(), 0);
		Entry.Instance->OnInventoryStackChangeDelegate().Unbind();
	}
	OnContainerEntryRemove(Entry, bIsForceRep); // 移除监听 Item Instance 变化
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
	// for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	// {
	// 	ContainerList.BroadcastRemoveEntryMessage(Entry, true);
	// }
	for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	{
		ContainerList.BroadcastAddEntryMessage(Entry);
	}
}

bool UInvSys_BaseEquipContainerObject::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	bool LOCAL_IsRemoveEquipContainer = Super::RemoveItemInstance(InItemInstance);
	if (LOCAL_IsRemoveEquipContainer)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("正在删除容器内的所有物品"))
		// 移除的是容器本体，需要将所有的物品打包转移。
		InItemInstance->MyInstances.Empty();
		InItemInstance->MyInstances.Reserve(ContainerList.Entries.Num());
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

bool UInvSys_BaseEquipContainerObject::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	bool bResult = false;
	check(InItemInstance);
	if (InItemInstance)
	{
		bResult |= Super::RestoreItemInstance(InItemInstance);
		if (InItemInstance->MyInstances.Num() > 0)
		{
			//如果内部存在其他物品则表明该物品是一个装备容器，那么就需要将内部物品全部转移进来。
			for (UInvSys_InventoryItemInstance* TempItemInstance : InItemInstance->MyInstances)
			{
				bResult |= AddItemInstance(TempItemInstance);
			}
		}
		else if(bResult == false)
		{
			//内部没有其他物品，且物品不会被装备，说明它只是一个普通的物品
			bResult |= AddItemInstance(InItemInstance);
		}
	}
	return bResult;
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
	if (ContainerLayout == nullptr)
	{
		auto ContainerLayoutFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		if (ContainerLayoutFragment)
		{
			ContainerLayout = CreateWidget<UInvSys_InventoryWidget>(GetWorld(), ContainerLayoutFragment->ContainerLayout);
			ContainerLayout->SetInventoryComponent(InventoryComponent);
			ContainerLayout->SetSlotTag(EquipSlotTag);
		}
	}
	if (EquipSlotWidget && InItemInstance)
	{
		//将布局控件的创建交给蓝图完成。
		if (EquipSlotWidget->IsA(UInvSys_EquipContainerSlotWidget::StaticClass()))
		{
			UInvSys_EquipContainerSlotWidget* ContainerSlotWidget = Cast<UInvSys_EquipContainerSlotWidget>(EquipSlotWidget);
			GetWorld()->GetTimerManager().SetTimerForNextTick([this, ContainerSlotWidget]()
			{
				if (ContainerSlotWidget->NS_ContainerGridLayout->HasAnyChildren() == false)
				{
					ContainerSlotWidget->NS_ContainerGridLayout->AddChild(ContainerLayout);
				}
				// 延迟到下一帧执行，确保ContainerLayout创建完成
				// ContainerLayout = ContainerSlotWidget->GetContainerLayoutWidget<UInvSys_InventoryWidget>();
				// UE_LOG(LogInventorySystem, Error, TEXT("尝试刷新容器所有物品"))
				TryRefreshContainerItems();
			});
		}
	}
}

void UInvSys_BaseEquipContainerObject::NativeOnUnEquipItemInstance()
{
	Super::NativeOnUnEquipItemInstance();
	ContainerLayout = nullptr;
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

UInvSys_InventoryWidget* UInvSys_BaseEquipContainerObject::GetContainerLayout(
	TSubclassOf<UInvSys_InventoryWidget> OutClass)
{
	return ContainerLayout;
}

bool UInvSys_BaseEquipContainerObject::ContainsItem(FGuid ItemUniqueID)
{
	return Super::ContainsItem(ItemUniqueID) || ContainerList.Contains(ItemUniqueID);
}

void UInvSys_BaseEquipContainerObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

}
