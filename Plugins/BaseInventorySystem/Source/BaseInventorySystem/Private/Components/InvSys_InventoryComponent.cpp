// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UInvSys_InventoryComponent::UInvSys_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
	
	// ...
}

void UInvSys_InventoryComponent::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem)
{
	if (InventoryObjectMap.Contains(NewItem.SlotName) == false)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("%s 必须在 InventoryObjectMap 中存在的。"), *NewItem.SlotName.ToString());
		return;
	}

	if (InventoryObjectMap[NewItem.SlotName]->IsA(UInvSys_BaseEquipmentObject::StaticClass()))
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[%s:%s] EquipSlot add item [%s] to [%s]."),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName(),
			*NewItem.ItemID.ToString(), *NewItem.SlotName.ToString());
		UInvSys_BaseEquipmentObject* EquipmentObj = Cast<UInvSys_BaseEquipmentObject>(InventoryObjectMap[NewItem.SlotName]);
		EquipmentObj->AddInventoryItemToEquipSlot(NewItem);
	}
}

bool UInvSys_InventoryComponent::HasAuthority() const
{
	ensure(GetOwner());
	return GetOwner() ? GetOwner()->HasAuthority() : false;
}

APlayerController* UInvSys_InventoryComponent::GetPlayerController() const
{
	ensure(OwningPlayer);
	return OwningPlayer;
}

bool UInvSys_InventoryComponent::IsContainsInventoryItem(const FName ItemUniqueID)
{
	for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		if (InventoryObject->ContainsItem(ItemUniqueID))
		{
			return true;
		}
	}
	return false;
}

bool UInvSys_InventoryComponent::IsLocalController() const
{
	ensure(OwningPlayer);
	return OwningPlayer ? OwningPlayer->IsLocalController() : false;
}

// Called when the game starts
void UInvSys_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (HasAuthority())
	{
		ConstructInventoryList();
	}
}

void UInvSys_InventoryComponent::NativeOnInitInventoryObjects(APlayerController* InController)
{
	OwningPlayer = InController;
	OnInitInventoryObjects(InController);
}

void UInvSys_InventoryComponent::ConstructInventoryList()
{
	// 权威服务器更新库存对象列表与映射关系
	if (InventoryContentMapping)
	{
		InventoryObjectList.Empty();
		InventoryObjectList.Reserve(InventoryContentMapping->InventoryContentList.Num());
		InventoryObjectMap.Empty();
		InventoryObjectMap.Reserve(InventoryContentMapping->InventoryContentList.Num());
		
		UE_LOG(LogInventorySystem, Log, TEXT("== 正在构建库存对象 [%s:%s] =="),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName());
		for (UInvSys_PreEditInventoryObject* PreEditInventoryObject : InventoryContentMapping->InventoryContentList)
		{
			if (PreEditInventoryObject == nullptr)
			{
				UE_LOG(LogInventorySystem, Log, TEXT("[%s] 库存对象内容映射 [InventoryContentMapping] 中存在空值."),
					HasAuthority() ? TEXT("Server") : TEXT("Client"));
				continue;
			}
			UInvSys_BaseInventoryObject* InventoryObject = PreEditInventoryObject->ConstructInventoryObject(this);
			// InventoryObjectList 会参与网络复制，故仅在权威服务器中添加新对象
			if (InventoryObject == nullptr)
			{
				UE_LOG(LogInventorySystem, Log, TEXT("[%s] 预设 [%s] 构建的库存对象为空."),
					HasAuthority() ? TEXT("Server") : TEXT("Client"), *PreEditInventoryObject->GetName());
				continue;
			}
							
			InventoryObjectList.Add(InventoryObject);
			InventoryObjectMap.Add(InventoryObject->GetSlotName(), InventoryObject);
			AddReplicatedSubObject(InventoryObject, COND_None);
		}
	}
}

UInvSys_PreEditInventoryObject* UInvSys_InventoryComponent::GetPreEditInventoryObject(int32 Index) const
{
	if (InventoryContentMapping && InventoryContentMapping->InventoryContentList.IsValidIndex(Index))
	{
		return InventoryContentMapping->InventoryContentList[Index];
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("GetPreEditInventoryObject 传入的 Index 无效。"))
	return nullptr;
}

void UInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_InventoryComponent, InventoryObjectList, COND_None);
}

void UInvSys_InventoryComponent::InitInventoryObj(APlayerController* NewPlayerController)
{
	if (bIsInitInventoryObjects)
	{
		return;
	}
	if (NewPlayerController && NewPlayerController->IsLocalController())
	{
		bIsInitInventoryObjects = true;
		InventoryObjectMap.Empty(InventoryObjectList.Num());

		NativeOnInitInventoryObjects(NewPlayerController);
		
		// OwningPlayer = NewPlayerController;
		// OnInitInventoryObjects(NewPlayerController);
		
		UE_LOG(LogInventorySystem, Log, TEXT("== 正在初始化库存对象 [%s:%s] =="),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName())
		for (int i = 0; i < InventoryObjectList.Num(); ++i)
		{
			check(InventoryObjectList[i]);
			InventoryObjectList[i]->InitInventoryObject(this, GetPreEditInventoryObject(i));
			InventoryObjectMap.Add(InventoryObjectList[i]->GetSlotName(), InventoryObjectList[i]);
		}
	}
}

void UInvSys_InventoryComponent::Client_TryRefreshInventoryObject_Implementation()
{
	for (UInvSys_BaseInventoryObject* InvObj : InventoryObjectList)
	{
		InvObj->RefreshInventoryObject("客户端数据与服务器数据不匹配，请求刷新客户端显示效果。");
	}
}

