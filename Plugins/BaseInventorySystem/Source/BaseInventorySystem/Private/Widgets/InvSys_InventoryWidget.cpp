// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryWidget.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/InvSys_InventoryHUD.h"

void UInvSys_InventoryWidget::InitInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	bInitInventoryWidget = true;
	check(NewInventoryObject)
	if (NewInventoryObject)
	{
		InventoryObject = NewInventoryObject;
		InventoryTag = InventoryObject->GetInventoryObjectTag();
		InventoryComponent = InventoryObject->GetInventoryComponent();
	}
	OnRefreshWidget();
}

void UInvSys_InventoryWidget::RemoveFromParent()
{
	UInvSys_InventoryHUD* InventoryHUD = UInvSys_InventorySystemLibrary::GetInventoryHUD(GetWorld());
	if (InventoryHUD)
	{
		InventoryHUD->RemoveWidget(InventoryTag);
	}
	Super::RemoveFromParent();
}

void UInvSys_InventoryWidget::NativeConstruct()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && InventoryObject == nullptr, LogInventorySystem, Error,
		TEXT("你必须在执行 Construct 函数前初始化 InventoryObject 属性."))
	if (InventoryObject && bInitInventoryWidget == false)
	{
		InitInventoryWidget(InventoryObject);
	}
	Super::NativeConstruct();
}

FGameplayTag UInvSys_InventoryWidget::GetSlotTag() const
{
	check(InventoryObject)
	return InventoryObject->GetInventoryObjectTag();
}

UInvSys_BaseInventoryObject* UInvSys_InventoryWidget::GetInventoryObject()
{
	check(InventoryObject);
	return InventoryObject;
}

