// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseContainerObject.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
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

void UInvSys_InventoryComponent::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem, FName TargetSlotName)
{
	if (InventoryObjectMap.Contains(TargetSlotName) == false)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("%s 必须在 InventoryObjectMap 中存在的。"), *TargetSlotName.ToString());
		return;
	}
	ensureMsgf(InventoryObjectMap[TargetSlotName]->IsA(UInvSys_BaseEquipmentObject::StaticClass()),
		TEXT("目标位置的库存对象必须是 UInvSys_BaseEquipmentObject 的子类。"));

	if (UInvSys_BaseEquipmentObject* EquipmentObj = Cast<UInvSys_BaseEquipmentObject>(InventoryObjectMap[TargetSlotName]))
	{
		EquipmentObj->AddInventoryItemToEquipSlot(NewItem, TargetSlotName);
	}
}

void UInvSys_InventoryComponent::AddInventoryItemToContainer(const FInvSys_InventoryItem& NewItem,
	UObject* ContainerObj)
{
	ensureMsgf(ContainerObj->IsA(UInvSys_BaseContainerObject::StaticClass()),
		TEXT("ContainerObj 必须是 UInvSys_BaseContainerObject 的子类。"));
}

void UInvSys_InventoryComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	/*for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		InventoryObject->PreReplication(ChangedPropertyTracker);
	}*/
}

void UInvSys_InventoryComponent::PostRepNotifies()
{
	Super::PostRepNotifies();
	
}

bool UInvSys_InventoryComponent::HasAuthority() const
{
	AActor* Owner = GetOwner();
	check(Owner);
	return Owner->HasAuthority();
}

APlayerController* UInvSys_InventoryComponent::GetPlayerController() const
{
	return OwningPlayer;
}

FTimerManager& UInvSys_InventoryComponent::GetWorldTimerManager() const
{
	AActor* Owner = GetOwner();
	check(Owner);
	return Owner->GetWorldTimerManager();
}

bool UInvSys_InventoryComponent::IsLocalController() const
{
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
			// 拷贝预设数据至新建对象中
			// InventoryObject->CopyPropertyFromPreEdit(this, PreEditInventoryObject);
							
			InventoryObjectList.Add(InventoryObject);
			InventoryObjectMap.Add(InventoryObject->GetSlotName(), InventoryObject);
			AddReplicatedSubObject(InventoryObject, COND_None);
		}
		// UE_LOG(LogInventorySystem, Log, TEXT("== 构建库存对象完成 =="))
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
	// DOREPLIFETIME_CONDITION(UInvSys_InventoryComponent, OwningPlayer, COND_OwnerOnly);
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
		OwningPlayer = NewPlayerController;
		InventoryObjectMap.Empty(InventoryObjectList.Num());

		OnInitInventoryObjects(NewPlayerController);
		
		UE_LOG(LogInventorySystem, Log, TEXT("== 正在初始化库存对象 [%s:%s] =="),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName())
		for (int i = 0; i < InventoryObjectList.Num(); ++i)
		{
			check(InventoryObjectList[i]);
			InventoryObjectList[i]->InitInventoryObject(this, GetPreEditInventoryObject(i));
			InventoryObjectMap.Add(InventoryObjectList[i]->GetSlotName(), InventoryObjectList[i]);
		}
		// UE_LOG(LogInventorySystem, Log, TEXT("== 初始化库存对象完成 [%s] =="), HasAuthority() ? TEXT("Server") : TEXT("Client"))
	}
}

void UInvSys_InventoryComponent::OnInitInventoryObjects_Implementation(APlayerController* NewPlayerController)
{
}

