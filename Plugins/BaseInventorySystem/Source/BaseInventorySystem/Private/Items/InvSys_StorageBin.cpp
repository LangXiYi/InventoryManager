// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_StorageBin.h"


// Sets default values
AInvSys_StorageBin::AInvSys_StorageBin()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInvSys_StorageBin::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInvSys_StorageBin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

