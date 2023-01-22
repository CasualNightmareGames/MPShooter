// Fill out your copyright notice in the Description page of Project Settings.


#include "MPShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MPShooter/Weapon/Weapon.h"
#include "MPShooter/MPShooterComponents/CombatComponent.h"

AMPShooterCharacter::AMPShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);


	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AMPShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMPShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AMPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMPShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMPShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMPShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMPShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMPShooterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMPShooterCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AMPShooterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AMPShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AMPShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AMPShooterCharacter::AimButtonReleased);

}

void AMPShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMPShooterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMPShooterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMPShooterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMPShooterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMPShooterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AMPShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AMPShooterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

}

void AMPShooterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMPShooterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AMPShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false); 
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AMPShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AMPShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMPShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}






