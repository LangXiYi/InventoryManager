// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/GridInvSys_GridEquipmentObject.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Widgets/GridInvSys_EquipmentSlotWidget.h"


UGridInvSys_GridEquipmentObject::UGridInvSys_GridEquipmentObject()
{
}

void UGridInvSys_GridEquipmentObject::TryRefreshOccupant(const FString& Reason)
{
	Super::TryRefreshOccupant(Reason);
	if (EquipmentSlotWidget && IsEquipped())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[%s] 接收到新的装备 [%s]"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *Occupant.ItemID.ToString())

		// todo:: Use interface?
		if (UGridInvSys_EquipmentSlotWidget* GridInvSys_EquipmentSlotWidget = Cast<UGridInvSys_EquipmentSlotWidget>(EquipmentSlotWidget))
		{
			GridInvSys_EquipmentSlotWidget->UpdateOccupant(Occupant);
		}
	}
}

void UGridInvSys_GridEquipmentObject::CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,
	UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(NewInventoryComponent, PreEditPayLoad);
	
	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipmentObject, EquipmentSlotWidgetClass);
	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipmentObject, EquipmentSupportType);
}

void UGridInvSys_GridEquipmentObject::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem)
{
	// 检查类型是否一致
	if (EquipmentSupportType == EGridInvSys_InventoryItemType::Weapon_Primary)
	{
		// 根据 NewItem.ItemID 判断物品类型
		// GetItemType(ItemID)
	}
	Super::AddInventoryItemToEquipSlot(NewItem);
}

void UGridInvSys_GridEquipmentObject::CreateDisplayWidget(APlayerController* PC)
{
	Super::CreateDisplayWidget(PC);

#if WITH_EDITOR
	if (InventoryComponent->IsA(UGridInvSys_InventoryComponent::StaticClass()) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("库存组件类型与填充的 InventoryObject 的类型不匹配。"))
		return;
	}
#endif
	UGridInvSys_InventoryComponent* GridInventoryComponent = static_cast<UGridInvSys_InventoryComponent*>(InventoryComponent);

	if (PC)
		EquipmentSlotWidget = CreateWidget<UGridInvSys_EquipmentSlotWidget>(PC, EquipmentSlotWidgetClass);
	else
		EquipmentSlotWidget = CreateWidget<UGridInvSys_EquipmentSlotWidget>(GetWorld(), EquipmentSlotWidgetClass);

	EquipmentSlotWidget->SetSlotName(GetSlotName());
	EquipmentSlotWidget->SetInventoryComponent(GetInventoryComponent());

	// 获取库存布局控件，并将装备槽插入指定位置
	UUserWidget* InventoryMenuWidget = GridInventoryComponent->GetInventoryLayoutWidget();
	if (InventoryMenuWidget == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("InventoryMenuWidget 在 GridInventoryComponent 中不存在"));
		return;
	}
	UNamedSlot* NamedSlot = Cast<UNamedSlot>(InventoryMenuWidget->GetWidgetFromName(GetSlotName()));
#if WITH_EDITOR
	if (NamedSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("命名槽[%s]在 InventoryMenuWidget 中不存在"), *GetSlotName().ToString())
		return;
	}
#endif

	NamedSlot->AddChild(EquipmentSlotWidget);
	// TryRefreshOccupant();
}

void UGridInvSys_GridEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
