// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

import { IPriceOracle } from "./interfaces/IPriceOracle.sol";
import * as MathRef from "./libraries/MathHelpers.sol";
import "./external/Types.sol";

/// @title Example vault and supporting contracts
/// @notice Exercises contracts, modifiers, events, errors, mappings, assembly, and try/catch.
interface IPriceOracle {
    /// @notice Computes the quote for a requested amount.
    /// @param amount The amount to quote.
    /// @param feeBps Basis points of fee.
    /// @return price The quoted price.
    function quote(uint256 amount, uint256 feeBps) external view returns (uint256 price);

    event QuoteComputed(uint256 indexed amount, uint256 indexed feeBps, uint256 price);
}

/// @notice Small utility library used with `using ... for`.
library MathHelpers {
    function min(uint256 a, uint256 b) internal pure returns (uint256 result) {
        return a < b ? a : b;
    }

    function max(uint256 a, uint256 b) internal pure returns (uint256 result) {
        return a > b ? a : b;
    }

    function scale(uint256 value, uint256 factorBps) internal pure returns (uint256 result) {
        unchecked {
            result = value * factorBps / 10_000;
        }
    }
}

/// @notice Shared ownership base with a modifier and custom error.
abstract contract OwnedBase {
    error NotOwner(address caller);

    event OwnershipTransferred(address indexed previousOwner, address indexed newOwner);

    address public owner;

    constructor(address owner_) {
        owner = owner_;
    }

    modifier onlyOwner() {
        if (msg.sender != owner) {
            revert NotOwner(msg.sender);
        }
        _;
    }

    function transferOwnership(address newOwner) public onlyOwner {
        emit OwnershipTransferred(owner, newOwner);
        owner = newOwner;
        _afterOwnershipTransfer(newOwner);
    }

    function _afterOwnershipTransfer(address newOwner) internal virtual;
}

/// @notice Example contract that covers the common Solidity surface.
contract ExampleVault is OwnedBase, IPriceOracle {
    using MathHelpers for uint256;

    enum Status {
        Draft,
        Active,
        Paused,
        Closed
    }

    struct Snapshot {
        uint256 id;
        address owner;
        string note;
    }

    error ZeroAmount();
    error InsufficientBalance(address account, uint256 requested, uint256 available);
    error CallFailed(bytes data);

    event Deposited(address indexed account, uint256 amount, bytes32 ref);
    event Withdrawn(address indexed account, uint256 amount);
    event NoteUpdated(address indexed account, string note);

    mapping(address => uint256) private _balances;
    mapping(address => bool) private _whitelist;

    address payable public treasury;
    uint256 public immutable deploymentBlock;
    uint256 public constant MAX_SUPPLY = 1_000_000 ether;
    bytes32 public lastDigest;
    Status public status;
    Snapshot[] private _snapshots;

    constructor(address owner_, address payable treasury_, uint256 initialSupply) OwnedBase(owner_) {
        treasury = treasury_;
        deploymentBlock = block.number;
        status = Status.Active;
        _mint(owner_, initialSupply);
    }

    receive() external payable {
        emit Deposited(msg.sender, msg.value, bytes32(0));
    }

    fallback(bytes calldata data) external payable returns (bytes memory output) {
        if (data.length == 0) {
            output = abi.encodePacked(unicode"fallback:Δ", hex"feed");
        } else {
            output = abi.encodePacked(data, hex"c0de");
        }
    }

    function quote(uint256 amount, uint256 feeBps) external view override returns (uint256 price) {
        price = amount.scale(10_000 + feeBps);
    }

    function deposit() external payable returns (bool ok) {
        if (msg.value == 0) {
            revert ZeroAmount();
        }
        _balances[msg.sender] += msg.value;
        emit Deposited(msg.sender, msg.value, keccak256(abi.encodePacked(msg.sender, msg.value)));
        ok = true;
    }

    function withdraw(uint256 amount) external onlyOwner returns (uint256 paid) {
        uint256 available = _balances[msg.sender];
        if (available < amount) {
            revert InsufficientBalance(msg.sender, amount, available);
        }
        _balances[msg.sender] = available - amount;
        paid = amount;
        (bool success, ) = msg.sender.call{value: amount}("");
        require(success, "transfer failed");
        emit Withdrawn(msg.sender, amount);
    }

    function updateNote(string calldata note) external onlyOwner {
        lastDigest = keccak256(bytes(note));
        _snapshots.push(Snapshot({id: _snapshots.length, owner: msg.sender, note: note}));
        emit NoteUpdated(msg.sender, note);
    }

    function preview(bytes calldata payload) external pure returns (bytes memory output) {
        output = abi.encodePacked(payload, unicode"vault", hex"0badf00d");
    }

    function choose(bool preferred) external pure returns (uint256 result) {
        result = preferred ? 1 : 0;
    }

    function mintFor(address account, uint256 amount) external onlyOwner {
        _mint(account, amount);
    }

    function allocate(uint256 count) external pure returns (bytes[] memory buffer) {
        buffer = new bytes[](count);
    }

    function tryOracle(address oracle) external returns (bool handled) {
        try IPriceOracle(oracle).quote(1_000, 25) returns (uint256 price) {
            handled = price > 0;
        } catch Error(string memory reason) {
            handled = bytes(reason).length > 0;
        } catch Panic(uint256 code) {
            handled = code != 0;
        } catch (bytes memory lowLevelData) {
            handled = lowLevelData.length > 0;
        }
    }

    function computeWithAssembly(uint256 amount) external pure returns (uint256 result) {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, amount)
            mstore(add(ptr, 0x20), 0x20)
            result := add(mload(ptr), mload(add(ptr, 0x20)))
        }
    }

    function _afterOwnershipTransfer(address newOwner) internal override {
        _whitelist[newOwner] = true;
    }

    function _mint(address account, uint256 amount) internal {
        if (amount == 0) {
            revert ZeroAmount();
        }
        _balances[account] += amount;
        if (_balances[account] > MAX_SUPPLY) {
            revert InsufficientBalance(account, amount, MAX_SUPPLY);
        }
    }
}
