#include "BattleCharacter.h" 
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" 
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Engine/Engine.h"
#include "MyProject/Core/MyGameInstance.h"

ABattleCharacter::ABattleCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    CurrentLevel = 1;
    CurrentExp = 0;
    MaxHP = 150.0f;
    CurrentHP = MaxHP;
    AttackDamage = 10.0f;
}

void ABattleCharacter::BeginPlay()
{
    Super::BeginPlay();

    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (GI)
    {
        if (GI->StoredHP > 0.0f)
        {
            CurrentLevel = GI->StoredLevel;
            MaxHP = GI->StoredMaxHP;
            CurrentHP = GI->StoredHP;
            AttackDamage = GI->StoredAttackDamage;
            CurrentExp = GI->StoredCurrentExp;
        }
        else
        {
            GI->StoredLevel = CurrentLevel;
            GI->StoredHP = CurrentHP;
            GI->StoredMaxHP = MaxHP;
            GI->StoredAttackDamage = AttackDamage;
            GI->StoredCurrentExp = CurrentExp;
        }
    }
}

void ABattleCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABattleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxisKey(EKeys::W, this, &ABattleCharacter::MoveForward);
    PlayerInputComponent->BindAxisKey(EKeys::S, this, &ABattleCharacter::MoveForward_Back);
    PlayerInputComponent->BindAxisKey(EKeys::D, this, &ABattleCharacter::MoveRight);
    PlayerInputComponent->BindAxisKey(EKeys::A, this, &ABattleCharacter::MoveRight_Left);

    PlayerInputComponent->BindAxisKey(EKeys::MouseX, this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxisKey(EKeys::MouseY, this, &ABattleCharacter::LookUp);

    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ABattleCharacter::Attack);
}

void ABattleCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ABattleCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ABattleCharacter::Attack()
{
    FVector Start = GetActorLocation();
    FVector Forward = GetActorForwardVector();
    FVector End = Start + (Forward * 150.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
                    FString::Printf(TEXT("Attack Hit! Damage: %.1f"), AttackDamage));
            }
        }
    }
}

void ABattleCharacter::LookUp(float Value)
{
    AddControllerPitchInput(-Value);
}

void ABattleCharacter::MoveForward_Back(float Value)
{
    MoveForward(-Value);
}

void ABattleCharacter::MoveRight_Left(float Value)
{
    MoveRight(-Value);
}

void ABattleCharacter::ApplyDamage(float DamageAmount)
{
    float ActualDamage = DamageAmount;
    if (bIsGuarding)
    {
        ActualDamage = DamageAmount * 0.5f;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Guarded! Damage Reduced."));
        }
    }

    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);
    bIsGuarding = false;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
            FString::Printf(TEXT("Damage: -%.1f"), ActualDamage));
    }

    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->StoredHP = CurrentHP;
    }

    if (CurrentHP <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character is Dead!"));
        if (GI) GI->ResetPlayerStats();
        UGameplayStatics::OpenLevel(this, FName("L_GameOver"));
    }
}

void ABattleCharacter::GainExp(int32 Amount)
{
    if (CurrentLevel >= MaxLevel) return;

    CurrentExp += Amount;

    while (CurrentLevel < MaxLevel && CurrentExp >= (CurrentLevel * 15))
    {
        CurrentExp -= (CurrentLevel * 15);
        
        LevelUp(); 
    }

    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->StoredLevel = CurrentLevel;
        GI->StoredCurrentExp = CurrentExp;
        GI->StoredMaxHP = MaxHP;
        GI->StoredHP = CurrentHP;
        GI->StoredAttackDamage = AttackDamage;

        UGameplayStatics::OpenLevel(this, FName("Map_Start"));
    }
}

void ABattleCharacter::LevelUp()
{
    CurrentLevel++;
    MaxHP += 20.0f;
    AttackDamage += 5.0f;
    CurrentHP = MaxHP;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT(" LEVEL UP! Now Level %d "), CurrentLevel));
    }
}

void ABattleCharacter::SetGuarding(bool bNewGuarding)
{
    bIsGuarding = bNewGuarding;

    if (GEngine && bIsGuarding)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Guard"));
    }
}