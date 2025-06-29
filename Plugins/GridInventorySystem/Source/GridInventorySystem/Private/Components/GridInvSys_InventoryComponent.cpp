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

void UGridInvSys_InventoryComponent::OnInitInventoryObjects_Implementation(APlayerController* NewPlayerController)
{
	/*if (NewPlayerController && NewPlayerController->IsLocalController())
	{

	}*/
	
	InventoryLayoutWidget = CreateWidget(NewPlayerController, InventoryLayoutWidgetClass);
}

void UGridInvSys_InventoryComponent::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem,
	FName TargetSlotName)
{
	Super::AddInventoryItemToEquipSlot(NewItem, TargetSlotName);
	/*UGridInvSys_GridEquipmentObject* GridEquipmentObj = Cast<UGridInvSys_GridEquipmentObject>(InventoryObjectMap[TargetSlotName]);
	ensureMsgf(GridEquipmentObj, TEXT("目标槽位 [%s] 的类型不是 UGridInvSys_GridEquipmentObject"), *TargetSlotName.ToString());
	if (GridEquipmentObj)
	{
		GridEquipmentObj->AddInventoryItemToEquipSlot(NewItem, TargetSlotName);
	}*/
}

// Called when the game starts
void UGridInvSys_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGridInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}