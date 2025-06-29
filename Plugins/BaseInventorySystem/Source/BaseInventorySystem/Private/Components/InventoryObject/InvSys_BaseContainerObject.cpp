// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseContainerObject.h"

#include "Net/UnrealNetwork.h"


void UInvSys_BaseContainerObject::AddDataToRep_AddedInventoryItems(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_AddedInventoryItems = true;
		Pending_AddedInventoryItems.Add(ItemUniqueID);
		UE_LOG(LogInventorySystem, Log, TEXT("[%s]Added Inventory Items."), *GetOwningActor()->GetName())
		TryRepInventoryItems_Add();
	}
}

void UInvSys_BaseContainerObject::AddDataToRep_RemovedInventoryItems(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_RemovedInventoryItems = true;
		Pending_RemovedInventoryItems.Add(ItemUniqueID);
		TryRepInventoryItems_Remove();
	}
}

void UInvSys_BaseContainerObject::AddDataToRep_ChangedInventoryItems(FName OldItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_ChangedInventoryItems = true;
		Pending_ChangedInventoryItems.Add(OldItemUniqueID);
		TryRepInventoryItems_Change();
	}
}

void UInvSys_BaseContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseContainerObject, ChangedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseContainerObject, AddedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseContainerObject, RemovedInventoryItems);
}

void UInvSys_BaseContainerObject::TryRepInventoryItems_Add()
{
	if (HasAuthority() == false || AddTimerHandle.IsValid()) return;
	
	AddTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		AddTimerHandle.Invalidate();
		// 延时处理标记，并确保上一帧未修改 Pending_AddedInventoryItems。
		if (bIsWait_Pending_AddedInventoryItems)
		{
			bIsWait_Pending_AddedInventoryItems = false;
			TryRepInventoryItems_Add();
			return;
		}
		
		AActor* OwningActor = GetOwningActor();
		check(OwningActor)
		if (OwningActor == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
			return;
		}
		AddedInventoryItems = Pending_AddedInventoryItems;
		OwningActor->ForceNetUpdate();

		Pending_AddedInventoryItems.Empty();
		if (GetNetMode() != NM_DedicatedServer && IsLocalController())
			OnRep_AddedInventoryItems();
		
		GetWorld()->GetTimerManager().SetTimer(AddTimerHandle, [this]() {
			if (!Pending_AddedInventoryItems.IsEmpty())
			{
				UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
				TryRepInventoryItems_Add(); // 存在新的需要更新的数据，强制继续更新。
			}
			AddedInventoryItems.Empty();
			AddTimerHandle.Invalidate();
		}, 0.1f, false);
	});	
}

void UInvSys_BaseContainerObject::TryRepInventoryItems_Remove()
{
	if (HasAuthority() == false || RemoveTimerHandle.IsValid()) return;
	
	RemoveTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		RemoveTimerHandle.Invalidate();
		// 延时处理标记，并确保上一帧未修改 Pending_RemovedInventoryItems。
		if (bIsWait_Pending_RemovedInventoryItems)
		{
			bIsWait_Pending_RemovedInventoryItems = false;
			TryRepInventoryItems_Remove();
			return;
		}
		
		AActor* OwningActor = GetOwningActor();
		check(OwningActor)
		if (OwningActor == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
			return;
		}
		RemovedInventoryItems = Pending_RemovedInventoryItems;
		OwningActor->ForceNetUpdate();

		Pending_RemovedInventoryItems.Empty();
		if (GetNetMode() != NM_DedicatedServer && IsLocalController())
			OnRep_RemovedInventoryItems();
		
		GetWorld()->GetTimerManager().SetTimer(RemoveTimerHandle, [this]() {
			if (!Pending_RemovedInventoryItems.IsEmpty())
			{
				UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
				TryRepInventoryItems_Remove(); // 存在新的需要更新的数据，强制继续更新。
			}
			RemovedInventoryItems.Empty();
			RemoveTimerHandle.Invalidate();
		}, 0.1f, false);
	});	
}

void UInvSys_BaseContainerObject::TryRepInventoryItems_Change()
{
	if (HasAuthority() == false || ChangeTimerHandle.IsValid()) return;

	ChangeTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		ChangeTimerHandle.Invalidate();
		// 延时处理标记，并确保上一帧未修改 Pending_ChangedInventoryItems。
		if (bIsWait_Pending_ChangedInventoryItems)
		{
			bIsWait_Pending_ChangedInventoryItems = false;
			TryRepInventoryItems_Change();
			return;
		}
		
		AActor* OwningActor = GetOwningActor();
		check(OwningActor)
		if (OwningActor == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
			return;
		}
		ChangedInventoryItems = Pending_ChangedInventoryItems;
		OwningActor->ForceNetUpdate();

		Pending_ChangedInventoryItems.Empty();
		if (GetNetMode() != NM_DedicatedServer && IsLocalController())
			OnRep_ChangedInventoryItems();
		
		GetWorld()->GetTimerManager().SetTimer(ChangeTimerHandle, [this]() {
			if (!Pending_ChangedInventoryItems.IsEmpty())
			{
				UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
				TryRepInventoryItems_Change(); // 存在新的需要更新的数据，强制继续更新。
			}
			ChangedInventoryItems.Empty();
			ChangeTimerHandle.Invalidate();
		}, 0.1f, false);
	});	
}

void UInvSys_BaseContainerObject::OnRep_ChangedInventoryItems()
{
	OnUpdatedContainerItems(ChangedInventoryItems);
}

void UInvSys_BaseContainerObject::OnRep_AddedInventoryItems()
{
	OnAddedContainerItems(AddedInventoryItems);
}

void UInvSys_BaseContainerObject::OnRep_RemovedInventoryItems()
{
	OnRemovedContainerItems(RemovedInventoryItems);
}