// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_FoodInstance.h"

#include "Net/UnrealNetwork.h"

UGridInvSys_FoodInstance::UGridInvSys_FoodInstance()
{
	bIsUsableItemInstance = true;
}

void UGridInvSys_FoodInstance::PreUpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance,
	int32 DeltaStackCount)
{
	Super::PreUpdateItemStackCount(ItemInstance, DeltaStackCount);
	if (ItemInstance->IsA<UGridInvSys_FoodInstance>())
	{
		auto Food = Cast<UGridInvSys_FoodInstance>(ItemInstance);
		int32 From = Food->FreshTime * DeltaStackCount;

		float NewFreshTime = (FreshTime * StackCount + From) / (StackCount + DeltaStackCount);
		SetFoodFreshTime(NewFreshTime);
	}
}

void UGridInvSys_FoodInstance::SetFoodFreshTime(float NewTime)
{
	FreshTime = NewTime;
	GetWorld()->GetTimerManager().SetTimer(FreshTimerHandle, [this]()
	{
		// 更新物品剩余新鲜时间
		FreshTime -= 1.f;
		if (FreshTime <= 0.f)
		{
			// todo::食物腐烂
			GetWorld()->GetTimerManager().ClearTimer(FreshTimerHandle);
		}
	}, 1.f, true);
}

void UGridInvSys_FoodInstance::InitInventoryItemInstance()
{
	Super::InitInventoryItemInstance();
	SetFoodFreshTime(FreshTime);
}

void UGridInvSys_FoodInstance::BeginDestroy()
{
	Super::BeginDestroy();
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FreshTimerHandle);
	}
}

void UGridInvSys_FoodInstance::NativeOnUseItemInstance()
{
	Super::NativeOnUseItemInstance();
	check(HasAuthority())

	UE_LOG(LogInventorySystem, Log, TEXT("Use Item Food: %f"), FreshTime)
	SetItemStackCount(StackCount - 1);
	if (StackCount <= 0)
	{
		RemoveAndDestroyFromInventory();
	}
}

void UGridInvSys_FoodInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_FoodInstance, FreshTime);
}
